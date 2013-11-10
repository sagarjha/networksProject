/** 
 * @file
 * 
 * @brief The Server Program
 * 
 * @details This sets up the server to serve file send and retireve requests, and coordinates with the user end to send and retrieve files.
 *
 * @author Ayush Kanodia
 *
 * @author Raghav Gupta
 *
 * @author Sagar Jha
 */
 
/* include files */
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include "config.h"
#include "time.h"

/** The client's TCP port number is explicitly randomly generated*/
#define PORTNUMTCP rand() % 10000 + 10000 
/** The maximum send and receive buffer size*/
#define BUFFERSIZE 1500

/** The handlers for immediate reuse of sockets*/
#ifndef ERESTART
#define ERESTART EINTR
#endif

/** The function which handles the send operation
 * @param socketD The socket descriptor
 * @param MD5sum The name of the file to be sent 
 * @param inputFolder The location of the file to be sent
 *
 * @retval 0 if send is successful, anything else otherwise
 */
int sendFile(int socketD, unsigned char* MD5sum,char* inputFolder);

/** The function which sets up the TCP connection to the client
 * @param remoteAddress The address of the client
 * @param The node ID of this server
 * 
 * @retval The socket descriptor if connection is successful, a negative value else otherwise
 */
 
int setupConnection(struct sockaddr_in remoteAddress, int selfId);

/** The function which handles the receive operation
 * @param socketD The TCP socket descriptor to receive files from  
 * @param MD5sum The name with which the file will be stored
 * @param outputFolder The name of the directory where the file needs to be stored
 *
 * @retval 0 if receive is successful, anything else otherwise
 */
int receiveFile(int socketD, unsigned char* MD5sum, char* outputFolder);

/** The main function.
 * This handles coordination.
 * See comments inline for details.
 */
int main (int argc, char* argv[]) {

  /* There should be exactly two command line arguments*/
  if (argc != 2) {
    printf("Usage: <executable> <id>");
    return -1;
  }

  /*extracting command line arguments*/
  int selfId = atoi(argv[1]);

  createMesh();

  if (selfId >= numNodes) {
    printf("server id out of range\n");
    return -1;
  }

  struct sockaddr_in selfAddressUDP = getAddress(selfId); //our UDP address
  
  /* Creating the UDP socket */
  int socketDUDP = 0;
  if ((socketDUDP = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
    {
      printf("cannot create socket\n");
      return -1;
    }

  if (bind(socketDUDP, (struct sockaddr *)&selfAddressUDP, sizeof(selfAddressUDP)) < 0) {
    printf("Bind failed\n");
    close(socketDUDP);
    return 0;
  }

  struct sockaddr_in remoteAddress; //remote address
  socklen_t addrlength = sizeof(remoteAddress); //size of address format
  unsigned char buffer[BUFFERSIZE]; //messages are received in this buffer
  
  /* Now waiting continuously for user requests */
  while (1) {
    printf("Listening\n");
    
    /* Listening for a UDP request packet*/
    int bytesReceived = recvfrom(socketDUDP, buffer, BUFFERSIZE, 0, (struct sockaddr *)&remoteAddress, &addrlength);

    /* socket error*/
    if(bytesReceived < 0)
      {
	printf("Socket Error, incoming message ignored\n");
	continue;
      }

    /* packet from user received now, extracting information from the packet */
    
    /* send vs retrieve */
    char option = buffer[0];
    
    /* MD5 sum */
    unsigned char MD5sum[16];
    memcpy(MD5sum, buffer+1, 16);

    /* Printing the MD5 sum */
    int i = 0;
    for (i = 0; i < 16; i++)
      printf ("%02x", MD5sum[i]);
  
    /* rem stores the md5sum modulo the number of nodes*/
    int rem = 0;
     
    /* each byte of the array is composed of two hexadecimal numbers*/
    for (i = 0; i < 16; i++) {
      int num = MD5sum[i];
      /* extract the first hex digit and update the remainder*/
      rem = ((rem*16) + (num/16))%numNodes;
      /* do the same with the second hex digit*/
      rem = ((rem*16) + (num%16))%numNodes;
    }
    printf("\nMD5 modulo number of nodes is %d\n",rem);

    /* The case where the request needs to be forwarded to another host*/
    if (rem != selfId) {  
      /* UDP address of the node to which the request is to be forwarded*/
      struct sockaddr_in serverAddressUDP = getAddress(rem); /* UDP address of the node to which the request is to be forwarded*/
    
      /* Forwarding Request */
      if(sendto(socketDUDP, buffer, bytesReceived, 0, (struct sockaddr *)&serverAddressUDP,  sizeof(serverAddressUDP)) < 0)
	{
	  printf("Sending Failed\n");
	  continue;
	}
    }
  
    /* The case where the request needs to be serviced */
    else {
      /* Decoding the message */
    
      /* Send vs Receive */
      int option = buffer[0];
      
      /* The remote TCP address */
      struct sockaddr_in remoteAddress;
      memcpy(&remoteAddress, buffer+17, sizeof(remoteAddress));
      
      /* Setting up TCP connection*/   
      int socketDTCP = setupConnection(remoteAddress, selfId);
      if(socketDTCP < 0)
	{
          continue;
	}
      
      /* File storage request*/
      if (option == '0') {
	/*Receive file*/
      	printf("File Storage Request\n");
	receiveFile (socketDTCP, MD5sum, getFolder(selfId));
	printf("File Received\n");
      }
    
      /* File retrieve request */
      else if (option == '1') {
	/*Send file*/
     	printf("File Retrieve Request\n");
	sendFile (socketDTCP, MD5sum, getFolder(selfId));
	printf("File Sent\n");
      }

      /*Shutdown TCP socket*/
      shutdown(socketDTCP, 2);
    }
  }

  return 0;
}

int sendFile(int socketD, unsigned char* MD5sum, char* inputFolder)
{
  /*nbytes keeps track of the number of bytes last sent*/
  int nbytes;

  /* File pointer to the file to be sent*/
  FILE *inputFile;
  
  /*storing file size in long*/
  int fileSize;
  char fullFileName[100];
  
  /*Calculating 32 bit file name from 16 bit MD5sum*/
  unsigned char MD5fileName[33];
  int i = 0;
  for (i = 0; i < 16; i++) {
    int num = MD5sum[i];
    int num1 = num/16;
    int num2 = num%16;
    MD5fileName [2*i] = num1+48+39*(num1/10);
    MD5fileName [2*i+1] = num2+48+39*(num2/10);
  }
  MD5fileName[32] = 0;

  /*Generating full file name from directory name and MD5 file name*/
  sprintf(fullFileName, "%s/%s", inputFolder, MD5fileName);

  /*Open Input File*/
  inputFile = fopen(fullFileName, "rb");
  if(inputFile == NULL)
    {
      printf("Could not open file to be sent\n");
      int temp = -1;
      nbytes = write(socketD, &temp, sizeof(int));
      return -1;
    }
  /*Calculate file size*/
  fseek(inputFile, 0, SEEK_END);
  fileSize = ftell(inputFile);
  fseek(inputFile, 0, SEEK_SET);

  int fileNameSize = 32;

  /*sending the file name size, which is not really required*/
  nbytes = write(socketD, &fileNameSize, sizeof(int));

  /*sending the file name, which is also not really required*/
  nbytes = write(socketD, MD5fileName, fileNameSize);

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
      printf("%f%% Sent\n", (double)countBytesSent * 100.0 / (double)fileSize);
    }
       
  fclose(inputFile);
  return 0;
} 

int setupConnection(struct sockaddr_in remoteAddress, int selfId)
{

  struct sockaddr_in myAddress = getAddress(selfId);
  
  /* assigning a port number */
  myAddress.sin_port = htons(PORTNUMTCP);

  /*create a TCP/IP socket*/
  int socketD;
  if((socketD = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
      printf("Socket Creation Unsuccessful\n");
      return -1;
    }

  /*attempting to bind the socket descriptor to this IP address and port (myAddress)*/
  if(bind(socketD, (struct sockaddr*)&myAddress, sizeof(myAddress)) < 0)
    {
      printf("Socket bind at user end unsuccessful\n");
      close(socketD);
      return -1;
    }

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

  /*Connection Successfully Established*/
  return socketD;
}

int receiveFile(int socketD, unsigned char* MD5sum, char* outputFolder)
{
  /*This variable indicates the number of bytes read*/

  int numBytes;
    
  /*Reading file name size*/
  int fileNameSize;
  numBytes = read(socketD, &fileNameSize, sizeof(int));
    
  /*Reading file name*/
  char fileName[fileNameSize + 1];
  numBytes = read(socketD, fileName, fileNameSize);
  fileName[fileNameSize] = 0;

  /*Reading file size*/
  int fileSize;
  numBytes = read(socketD, &fileSize, sizeof(int));

  /*File Pointer*/
  FILE* outputFile;
  char fullFileName[100];
    
  /*Calculating 32 bit MD5 sum from 16 bit sum, this is the actual name with which the file needs to be stored*/
  unsigned char MD5fileName[33];
  int i = 0;
  for (i = 0; i < 16; i++) {
    int num = MD5sum[i];
    int num1 = num/16;
    int num2 = num%16;
    MD5fileName [2*i] = num1+48+39*(num1/10);
    MD5fileName [2*i+1] = num2+48+39*(num2/10);
  }
  MD5fileName[32] = 0;

  /*Generating the complete file name along with the location*/
  sprintf(fullFileName, "%s/%s", outputFolder, MD5fileName);
  printf ("Complete file name %s\n", fullFileName);
    
  /*Opening output file*/
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
  return 0;
}
