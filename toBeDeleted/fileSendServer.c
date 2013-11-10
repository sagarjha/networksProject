/** @file fileSendServer.c
 * @author Kanodia:Ayush
 * @author Gupta:Raghav
 * @author Jha:Sagar
 * @brief enacts the program which receives file from a user
 * @details later 
 */

/*#include(s)*/
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/errno.h>
#include <sys/wait.h>
#define SELFADDRESS "127.0.0.1"
#define SELFPORTNUM 3333

#ifndef ERESTART
#define ERESTART EINTR
#endif


/**
 * The function to setup the connection between the user and the server
 */
int setupService(char* selfAddress, int selfPortNum);
int receiveFile(int socketD);
    
int main(int argc, char* argv[])
{
    /*The socket descriptor for the tcp connection*/
    int fd; 
    char* selfAddress = SELFADDRESS;
    int selfPortNumber = SELFPORTNUM;

    setupService(selfAddress, selfPortNumber);
    return 0;
}  

int setupService(char* selfAddress, int selfPortNum)
{
    int serviceSocket;
    int requestSocket;
    
    /*Create service socket*/
    if((serviceSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Cannot create socket\n");
        return -1;
    }

    /*Settings for immediate reuse of the socket*/
    int sockoptval = 1;
    setsockopt(serviceSocket, SOL_SOCKET, SO_REUSEADDR, &sockoptval, sizeof(int));

    struct sockaddr_in myAddress;
    socklen_t addrLength;

    memset((char*)&myAddress, 0, sizeof(myAddress));
    myAddress.sin_family = AF_INET;
    myAddress.sin_port = htons(selfPortNum);

    if (inet_aton(selfAddress, &myAddress.sin_addr)==0) 
     {
        printf("Address allocation failed\n");
        return(-1);
     }

    /* bind the socket to the ip address and port number */
    if(bind(serviceSocket, (struct sockaddr *)&myAddress, sizeof(myAddress)) < 0)
    {
        printf("Listener Socket Bind Failed");
        return(-1);
    }

    /*setup the socket for listening upto a queue length of 10*/
    if(listen(serviceSocket, 10) < 0)
    {
        printf("Listen Failed");
        return (-1);
    }

    /*Server Listening Started Message*/
    printf("Server started, IP %s, Port Number %d\n", selfAddress, selfPortNum);

    struct sockaddr_in remoteAddress;
    addrLength = sizeof(remoteAddress);

    /*Waiting in an infinite loop*/
    for(;;)
    {
        printf("Request Initiated\n");
        while((requestSocket = accept(serviceSocket, (struct sockaddr *)&remoteAddress, &addrLength)) < 0)
        { 
           /*Restart listening in the case of a special connection*/
           if((errno != ECHILD) && (errno != ERESTART) && (errno != EINTR))
           {
               printf("Accept Failed\n");
               return (-1);
           }
        }
        

        printf("Connection Received from IP %s, Port# %d\n", inet_ntoa(remoteAddress.sin_addr), ntohs(remoteAddress.sin_port));
        receiveFile(requestSocket);
        //shutdown initiated, not necessary
        shutdown(requestSocket, 2);
    
    }
}

int receiveFile(int socketD)
{
    /*This variable indicates the number of bytes read*/

    int numBytes;
    
    /*Reading file name size*/
    int fileNameSize;
    numBytes = read(socketD, &fileNameSize, sizeof(int));
    printf("Filename size %d\n", fileNameSize);
    
    /*Reading file name*/
    char fileName[fileNameSize + 1];
    numBytes = read(socketD, fileName, fileNameSize);
    fileName[fileNameSize] = 0;
    printf("Filename %s\n", fileName);

    /*Reading file size*/
    int fileSize;
    
    numBytes = read(socketD, &fileSize, sizeof(int));
    printf("Size of File %d bytes\n", fileSize);

    /*Writing to file*/
    FILE* outputFile;
    char fullFileName[50];
    sprintf(fullFileName, "receiveDir/%s", fileName);
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
    }

    fclose(outputFile);
    printf("File Received\n");     
    return 0;
}
