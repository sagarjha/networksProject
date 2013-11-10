/** 
 * @file
 * 
 * @brief The User Program
 * 
 * @details This sets up the user to send file send and retireve requests, and coordinates with the server end to send and retrieve files.
 *
 * @author Ayush Kanodia
 *
 * @author Raghav Gupta
 *
 * @author Sagar Jha
 */

/* include files */
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include <time.h>
#include <errno.h>
#include "mddriver.c"

/** The user's UDP port number is explicitly randomly generated*/
#define PORTNUMUDP rand() % 10000 + 10000
/** The user's TCP port number is explicitly randomly generated*/
#define PORTNUMTCP rand() % 10000 + 10000
/** The maximum send and receive buffer size*/
#define BUFFERSIZE 1500

/** The handlers for immediate reuse of sockets*/
#ifndef ERESTART
#define ERESTART EINTR
#endif

/** The function which handles the receive operation
 * @param socketD The TCP socket descriptor to receive files from  
 * @param fileName The name with which the file will be stored
 * @param fileNameSize The number of characters in the file name
 * @param dir The name of the directory where the file needs to be stored
 *
 * @retval 0 if receive is successful, anything else otherwise
 */
int receiveFile(int socketD, char* fileName, int fileNameSize, char* dir);

/** The function which handles the send operation
 * @param socketD The socket descriptor
 * @param fileName The complete file location of the file to be sent
 * @param fileNameSize The number of characters in fileName
 *
 * @retval 0 if send is successful, anything else otherwise
 */
int sendFile(int socketD, char fileName[100], int fileNameSize);

/** Returns the size of a null terminated string
 * @param the input string
 *
 * @retval the size of the string
 */
int getSize (char* str);

/** The main function.
 * This handles coordination.
 * See comments inline for details.
 */
int main(int argc, char* argv[])
{
    /* Initializing the Random Number Generator*/
    srand(time(NULL));

    /* There should be exactly seven command line arguments*/
    
    if(argc != 7)
    {
        printf("Usage: \n<executable> \n<0 for send, 1 for retrieve> \n<filename (only filename, not location) for send/md5sum for retrieve> \n<directory ending with a '/', which is the location of the file to be sent or the location at which it needs to be retrieved> \n<remoteIP> \n<remote Port Number> \n<selfIP>\n");
        return -1;
    }
    
    /*extracting command line arguments*/
   
    int option = atoi(argv[1]);
    if(option != 0 && option != 1)
    {
        printf("Invalid Option\n");
        return -1;
    }
    char* fileName = argv[2];
    char* directory = argv[3];
    char* remoteIP = argv[4];
    int remotePortNum = atoi(argv[5]);
    char *myAddress = argv[6]; 

    /*Opening the UDP socket*/
    struct sockaddr_in selfAddressUDP; //our UDP address
    struct sockaddr_in remoteAddress; //remote address
    socklen_t addrlength = sizeof(selfAddressUDP); //size of address format
    int bytesReceived; //number of bytes read (of incoming data)
    int socketDUDP; //our socket descriptor

    /* Creating the UDP socket */
    if ((socketDUDP = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("cannot create socket\n");
        return -1;
    }

    /* bind the socket to a valid IP address and a specific port */
    memset((char *)&selfAddressUDP, 0, sizeof(selfAddressUDP));
    selfAddressUDP.sin_family = AF_INET;
    if (inet_aton(myAddress, &selfAddressUDP.sin_addr)==0)
    {
        printf("Address allocation failed\n");
        return(0);
    }
    selfAddressUDP.sin_port = htons(PORTNUMUDP);

    if (bind(socketDUDP, (struct sockaddr *)&selfAddressUDP, sizeof(selfAddressUDP)) < 0)
    {
        printf("Bind failed\n");
        close(socketDUDP);
        return 0;
    }

    /* defining the remote address */
    memset((char *) &remoteAddress, 0, sizeof(remoteAddress));
    remoteAddress.sin_family = AF_INET;
    remoteAddress.sin_port = htons(remotePortNum);
    if (inet_aton(remoteIP, &remoteAddress.sin_addr)==0)
    {
        fprintf(stderr, "inet_aton() failed\n");
        return(-1);
    }

    /* opening TCP/IP socket */
    struct sockaddr_in selfAddressTCP;

    /*create a TCP/IP socket*/
    int socketDTCP;
    if((socketDTCP = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Socket Creation Unsuccessful\n");
        return -1;
    }

    /*Settings for immediate reuse of the socket*/
    int sockoptval = 1;
    setsockopt(socketDTCP, SOL_SOCKET, SO_REUSEADDR, &sockoptval, sizeof(int));


    /*Connect from a random IP*/
    memset((char*)&selfAddressTCP, 0, sizeof(selfAddressTCP));
    selfAddressTCP.sin_family = AF_INET;
    if (inet_aton(myAddress, &selfAddressTCP.sin_addr)==0)
    {
        printf("Address allocation failed\n");
        return(-1);
    }
    selfAddressTCP.sin_port = htons(PORTNUMTCP);

    /*attempting to bind the socket descriptor to this IP address and port (myAddress)*/
    if(bind(socketDTCP, (struct sockaddr*)&selfAddressTCP, sizeof(selfAddressTCP)) < 0)
    {
        printf("Socket bind at user end unsuccessful\n");
        close(socketDTCP);
        return -1;
    }

    /*setup the TCP socket for listening upto a queue length of 10*/
    if(listen(socketDTCP, 10) < 0)
    {
        printf("Listen Failed");
        return (-1);
    }

    /*now sending the message*/
    printf("Sending packet to %s port %d\n", remoteIP, remotePortNum);

    unsigned char buffer[BUFFERSIZE]; // messages are received in this buffer
    //send vs retrieve
    if(option == 0)
    {
        unsigned char MD5sum[16];
        /* Building the complete file name*/
        char fullFileName[100];
        strcpy (fullFileName, directory);
        strcat (fullFileName, fileName);

        /* Getting fileName size*/
        int fileNameSize = getSize (fileName);

        /* find the md5 sum and store in 'MD5sum'*/
        MDFile(fullFileName, MD5sum);

        /* building the message to be sent*/
        /* First the option */
        buffer[0] = '0';
        /* Then the MD5 sum*/
        memcpy(buffer + 1, MD5sum, 16);
        /* Then the TCP socket information*/
        memcpy(buffer + 17, &selfAddressTCP, sizeof(selfAddressTCP));
        /* Number of bytes to be sent*/
        int sizeSent = 17 + sizeof(selfAddressTCP);
        /* Sending */ 
        if(sendto(socketDUDP, buffer, sizeSent, 0, (struct sockaddr *)&remoteAddress,  sizeof(remoteAddress)) < 0)
        {
            printf("Sending Failed, Error # %d\n", errno);
            return(-1);
        }
        /*Waiting in an infinite loop*/
        for(;;)
        {
            printf("File Transfer Request Initiated\n");
            /*TCP transfer socket*/
            int requestSocket = 0;
            int addrLength = sizeof(remoteAddress);
            /*Waiting for incoming connection*/
            while((requestSocket = accept(socketDTCP, (struct sockaddr *)&remoteAddress, &addrLength)) < 0)
            {
                /*Restart listening in the case of a special connection*/
                if((errno != ECHILD) && (errno != ERESTART) && (errno != EINTR))
                {
                    printf("Accept Failed\n");
                    return (-1);
                }
            }

            /*Connection received*/
            printf("Connection Received from IP %s, Port# %d\n", inet_ntoa(remoteAddress.sin_addr), ntohs(remoteAddress.sin_port));
            /*Send the file*/
            sendFile(requestSocket, fullFileName, getSize(fileName));
            /*Shutdown the sockets*/
            shutdown(requestSocket, 2);
            shutdown(socketDTCP, 2);
            shutdown(socketDUDP, 2);
            return 0;
        }
    }

    else if(option == 1)
    {
        /*Getting file size*/
        int i, size = getSize (fileName);
        if (size != 32)
        {
            printf("Invalid MD5sum\n");
            return -1;
        }

        unsigned char MD5sum[16];
        
        /*Converting 32 character MD5 sum to 16 characters*/
        for (i = 0; i < 16; ++i)
        {
            MD5sum[i] = (fileName[2*i] - 48 - 39 * (fileName[2*i]/60))*16 + fileName[2*i+1] - 48 - 39 * (fileName[2*i+1]/60);
        }

        /*Building the send message*/
        /*Retrieve option*/
        buffer[0] = '1';
        /*Now the MD5 sum*/
        memcpy(buffer + 1, MD5sum, 16);
        /*Now the address for the TCP socket on which the user program is listening*/
        memcpy(buffer + 17, &selfAddressTCP, sizeof(selfAddressTCP));
        /*Number of bytes to be sent*/
        int sizeSent = 17 + sizeof(selfAddressTCP);
        if(sendto(socketDUDP, buffer, sizeSent, 0, (struct sockaddr *)&remoteAddress,  sizeof(remoteAddress)) < 0)
        {
            printf("Sending Failed\n");
            // fclose (fin);
            return(-1);
        }
        /*Waiting in an infinite loop*/
        for(;;)
        {
            printf("File Transfer Request Initiated\n");
            /*The socket on which the transfer will take place*/
            int requestSocket = 0;
            int addrLength = sizeof(remoteAddress);
            /*Waiting for incoming connection*/
            while((requestSocket = accept(socketDTCP, (struct sockaddr *)&remoteAddress, &addrLength)) < 0)
            {
                /*Restart listening in the case of a special connection*/
                if((errno != ECHILD) && (errno != ERESTART) && (errno != EINTR))
                {
                    printf("Accept Failed\n");
                    return (-1);
                }
            }

            /*Connection Received*/
            printf("Connection Received from IP %s, Port# %d\n", inet_ntoa(remoteAddress.sin_addr), ntohs(remoteAddress.sin_port));
            /*Retrieve file*/
            receiveFile(requestSocket, fileName, getSize(fileName), directory);
            /*Shutdown sockets*/
            shutdown(requestSocket, 2);
            shutdown(socketDTCP, 2);
            shutdown(socketDUDP, 2);
            return (0);
        }
    }

    return 0;
}

int sendFile(int socketD, char fileName[100], int fileNameSize)
{
    /*nbytes keeps track of the number of bytes last sent*/
    int nbytes;

    /*File pointer for file to be sent*/
    FILE *inputFile;

    /*Size of file to be sent*/
    int fileSize;

    /*Complete file name*/
    char fullFileName[100];
    sprintf(fullFileName, "%s", fileName);
    
    /*Open file*/
    inputFile = fopen(fullFileName, "rb");
    if(inputFile == NULL)
    {
        printf("Could not open file to be sent\n");
        return -1;
    }

    /*Calculate File Size*/
    fseek(inputFile, 0, SEEK_END);
    fileSize = ftell(inputFile);
    fseek(inputFile, 0, SEEK_SET);

    /*sending the file name size*/
    nbytes = write(socketD, &fileNameSize, sizeof(int));

    /*sending the file name*/
    nbytes = write(socketD, fileName, fileNameSize);

    /*sending the file size*/
    nbytes = write(socketD, &fileSize, sizeof(int));

    /*sending the file contents*/
    char* fileBuffer[1500];
    int countBytesRead = 0;
    int countBytesSent = 0;
    /*sending in bursts of 1500 bytes*/
    while(countBytesSent < fileSize)
    {
        int currentCountBytesRead = 0;
        currentCountBytesRead += fread(fileBuffer, 1, 1500, inputFile);
        int currentBytesSent = 0;
        while(currentBytesSent != currentCountBytesRead)
        {
            currentBytesSent += write(socketD, fileBuffer, (currentCountBytesRead-currentBytesSent));
        }
        countBytesRead += currentCountBytesRead;
        countBytesSent += currentBytesSent;
        printf("%f%% sent\n", (double)countBytesSent * (double)100/ (double)fileSize);
    }

    fclose(inputFile);
    printf("File Sent\n");
    return 0;
}


int receiveFile(int socketD, char* fileNameInput, int fileNameSize, char* dir)
{
    /*This variable indicates the number of bytes read*/

    int numBytes;

    /*Reading file name size*/
    int temp;
    numBytes = read(socketD, &temp, sizeof(int));
    
    /*Size = -1 indicates file not found on server*/
    if (temp == -1)
    {
        printf("File not found\n");
        return -1;
    }

    /*Reading file name, this is not really used, since we already have the MD5 sum at the user which is used for the file name*/
    char fileName[temp + 1];
    numBytes = read(socketD, fileName, temp);
    fileName[temp] = 0;

    /*Reading file size*/
    int fileSize;
    numBytes = read(socketD, &fileSize, sizeof(int));

    /*Opening file*/
    FILE* outputFile;
    char fullFileName[100];
    sprintf(fullFileName, "%s%s", dir, fileNameInput);
    outputFile = fopen(fullFileName, "wb");
    if(outputFile == NULL)
    {
        printf("Could not open output file\n");
        return -1;
    }
    int bytesWritten = 0;
    char* writeBuffer[1500];
    /*Reading in bursts of 1500 bytes*/
    while(bytesWritten < fileSize)
    {
        int currentBytesWritten = 0;
        currentBytesWritten += read(socketD, writeBuffer, 1500);
        fwrite(writeBuffer, 1, currentBytesWritten, outputFile);
        bytesWritten += currentBytesWritten;
        printf("%f%% received\n", (double)bytesWritten*100.0/(double)fileSize);
    }

    fclose(outputFile);
    printf("File Received\n");
    return 0;
}


int getSize (char* str)
{
    /*Counting till the first null character and returning the size*/
    int count = 0;
    while (str[count++] != '\0')
    {

    }
    return count-1;
}
