/** @file fileSendUser.c
 * @author Kanodia:Ayush
 * @author Gupta:Raghav
 * @author Jha:Sagar
 * @brief enacts the program which sends file to a server
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
#define SELFADDRESS "127.0.0.2"
#define REMOTEADDRESS "127.0.0.1"
#define REMOTEPORTNUM 3333


/**
 * The function to setup the connection between the user and the server
 */
int setupConnection(char *server, int portNum);
int disconnect(int socketD);
int sendFile(int socketD, char fileName[50], int fileNameSize);

int main(int argc, char* argv[])
{
    /*The socket descriptor for the tcp connection*/
    int fd; 
    char* server = REMOTEADDRESS;
    int remotePortNumber = REMOTEPORTNUM;

    /*creating the socket*/
    if((fd = setupConnection(server, remotePortNumber)) < 0)
    {
        printf("Socket Creation Unsuccessul, Exiting... \n");
        return -1;
    }

    char fileName[50];
    sprintf(fileName, "sampleFile.txt\0");
    int fileSending = sendFile(fd, fileName, 14);
    disconnect(fd);
    return 0;
  

}

int setupConnection(char *server, int portNum)
{
    printf("Server %s Port# %d \n", server, portNum);
    struct sockaddr_in myAddress;
    char *selfAddress = SELFADDRESS;
    char *serverAddress = REMOTEADDRESS;

    /*create a TCP/IP socket*/
    int socketD;
    if((socketD = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Socket Creation Unsuccessful\n");
        return -1;
    }

    /*Connect from a random IP*/
    memset((char*)&myAddress, 0, sizeof(myAddress));
    myAddress.sin_family = AF_INET;
     if (inet_aton(selfAddress, &myAddress.sin_addr)==0) 
     {
        printf("Address allocation failed\n");
        return(-1);
     }
    myAddress.sin_port = htons(0);

    /*attempting to bind the socket descriptor to this IP address and port (myAddress)*/
    if(bind(socketD, (struct sockaddr*)&myAddress, sizeof(myAddress)) < 0)
    {
        printf("Socket bind at user end unsuccessful\n");
        close(socketD);
        return -1;
    }

    /*get self address information into myAddress*/
    unsigned int addrLength;
    addrLength = sizeof(myAddress);
    if(getsockname(socketD, (struct sockaddr*)&myAddress, &addrLength) < 0)
    {
        printf("Socket Information not obtained\n");
        close(socketD);
        return -1;
    }
    /*Printing port number*/
    printf("Port Number : %d\n", ntohs(myAddress.sin_port));

    /*now creating the server's address
     * adding the server's IP and socket information*/

    struct sockaddr_in remoteAddress;
    memset((char*)&remoteAddress, 0, sizeof(remoteAddress));
    remoteAddress.sin_family = AF_INET;
    if (inet_aton(server, &remoteAddress.sin_addr)==0) 
    {
        printf("Remote address allocation failed\n");
        return(-1);
    }
    remoteAddress.sin_port = htons(portNum);

    printf("%d remote Port Number, %s remote IP Address\n", portNum, server);
    /*Trying to establish connection*/
    if(connect(socketD, (struct sockaddr*)&remoteAddress, sizeof(remoteAddress)) < 0)
    {
        printf("Connection to server failed\n");
        return -1;
    }
    else
    {
        printf("Connection to server succeeded\n");
    }

    return socketD;
}

int disconnect(int socketD)
{
    shutdown(socketD, 2);
    return 0;
}

int sendFile(int socketD, char fileName[50], int fileNameSize)
{
    /*nbytes keeps track of the number of bytes last sent*/
    int nbytes;

    FILE *inputFile;
    /*storing file size in long*/
    int fileSize;
    char fullFileName[50];
    sprintf(fullFileName, "sendDir/%s", fileName);
    inputFile = fopen(fullFileName, "rb");
    if(inputFile == NULL)
    {
        printf("Could not open file to be sent\n");
        return -1;
    }
    fseek(inputFile, 0, SEEK_END);
    fileSize = ftell(inputFile);
    printf("Size = %d\n", fileSize);
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
        printf("Bytes Sent %d\n", countBytesSent);
    }
       
    fclose(inputFile);
    printf("File Sent\n");

    return 0;
} 


