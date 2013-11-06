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

#define PORTNUMTCP 9315
#define BUFFERSIZE 1500
#ifndef ERESTART
#define ERESTART EINTR
#endif

int sendFile(int socketD, char fileName[50], int fileNameSize);
int setupConnection(struct sockaddr_in remoteAddress, int selfId);

int main (int argc, char* argv[]) {
  if (argc != 2) {
    printf("Usage: <executable> <id>");
    return -1;
  }

  int selfId = atoi(argv[1]);

  createMesh();

  struct sockaddr_in selfAddressUDP = getAddress(selfId); //our UDP address
  
  int socketDUDP = 0;
  /* Creating the UDP socket */
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
  int bytesReceived = recvfrom(socketDUDP, buffer, BUFFERSIZE, 0, (struct sockaddr *)&remoteAddress, &addrlength);

  //socket error
  if(bytesReceived < 0)
    {
      printf("Socket Error, incoming message ignored\n");
    }

  // packet from user
  char option = buffer[0];
  unsigned char MD5sum[16];
  memcpy(MD5sum, buffer+1, 16);

  int i = 0;
  for (i = 0; i < 16; i++)
    printf ("%02x", MD5sum[i]);
  
  // rem gives the md5sum remainder by number of nodes
  int rem = 0;
     
  // each byte of the array is composed of two hexadecimal numbers
  for (i = 0; i < 16; i++) {
    int num = MD5sum[i];
    // extract the first hex digit and update the remainder
    rem = ((rem*16) + (num/16))%numNodes;
    // do the same with the second hex digit
    rem = ((rem*16) + (num%16))%numNodes;
  }
  printf("%d\n",rem);

  if (rem != selfId) {  
    struct sockaddr_in serverAddressUDP = getAddress(rem); //our UDP address

    if(sendto(socketDUDP, buffer, bytesReceived, 0, (struct sockaddr *)&serverAddressUDP,  sizeof(serverAddressUDP)) < 0)
      {
	printf("Sending Failed\n");
	return(-1);
      }
  }
  
  else {
    int option = buffer[0];
    struct sockaddr_in remoteAddress;
    memcpy(&remoteAddress, buffer+17, sizeof(remoteAddress));
    int socketDTCP = setupConnection(remoteAddress, selfId);
    
  }
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

int setupConnection(struct sockaddr_in remoteAddress, int selfId)
{
  // printf("Server %s Port# %d \n", server, portNum);
  struct sockaddr_in myAddress = getAddress(selfId);

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

  /* /\*get self address information into myAddress*\/ */
  /* unsigned int addrLength; */
  /* addrLength = sizeof(myAddress); */
  /* if(getsockname(socketD, (struct sockaddr*)&myAddress, &addrLength) < 0) */
  /*   { */
  /*     printf("Socket Information not obtained\n"); */
  /*     close(socketD); */
  /*     return -1; */
  /*   } */

  /*now creating the server's address
   * adding the server's IP and socket information*/

  /* struct sockaddr_in remoteAddress; */
  /* memset((char*)&remoteAddress, 0, sizeof(remoteAddress)); */
  /* remoteAddress.sin_family = AF_INET; */
  /* if (inet_aton(server, &remoteAddress.sin_addr)==0)  */
  /*   { */
  /*     printf("Remote address allocation failed\n"); */
  /*     return(-1); */
  /*   } */
  /* remoteAddress.sin_port = htons(portNum); */

  /* printf("%d remote Port Number, %s remote IP Address\n", portNum, server); */


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
