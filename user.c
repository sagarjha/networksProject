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
#include "mddriver.c"

//!!!!confirm this later !!!!
#define SELFADDRESS "127.0.0.101"
#define PORTNUMUDP rand() % 10000 + 10000   
#define PORTNUMTCP rand() % 10000 + 10000
#define BUFFERSIZE 1500
#ifndef ERESTART
#define ERESTART EINTR
#endif

int receiveFile(int socketD, char* fileName, int fileNameSize, char* dir);
int sendFile(int socketD, char fileName[50], int fileNameSize);
void extractFileName(char* justName, int* fileNameLength, char* fileName, int fileNameSize);
int getSize (char* str);
int findMap (unsigned char* MD5sum, char* fileName);

int main(int argc, char* argv[])
{
  srand(time(NULL));
  /* FILE *fin; */
  /* fin = fopen("name-md5sum-map","a+"); */
  if(argc != 6)
    {
      printf("Usage: <executable> <0 for send, 1 for retrieve> <md5sum> <directory ending with /> <remoteIP> <remote Port Number>\n");
      /* fclose (fin); */
      return -1;
    }

  int option = atoi(argv[1]);
  if(option != 0 && option != 1)
    {
      printf("Invalid Option\n");
      // fclose (fin);
      return -1;
    }
  char* fileName = argv[2];
  char* directory = argv[3];
  char* remoteIP = argv[4];
  int remotePortNum = atoi(argv[5]);
  //extracting only the file name 
  //!!!!! COMPLETE THIS !!!!!!

  /*Opening the UDP socket*/
  struct sockaddr_in selfAddressUDP; //our UDP address
  struct sockaddr_in remoteAddress; //remote address
  socklen_t addrlength = sizeof(selfAddressUDP); //size of address format
  int bytesReceived; //number of bytes read (of incoming data)
  int socketDUDP; //our socket descriptor
  int msgsRcvd; //number of messages received by the server since it started listening
  char *myAddress = SELFADDRESS; // The address of the client

  /* Creating the UDP socket */
  if ((socketDUDP = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
    {
      printf("cannot create socket\n");
      // fclose (fin);
      return -1;
    }

  /* bind the socket to any valid IP address and a specific port */
  memset((char *)&selfAddressUDP, 0, sizeof(selfAddressUDP));
  selfAddressUDP.sin_family = AF_INET;
  if (inet_aton(myAddress, &selfAddressUDP.sin_addr)==0) {
    printf("Address allocation failed\n");
    // fclose (fin);
    return(0);
  }	

  selfAddressUDP.sin_port = htons(PORTNUMUDP);

  if (bind(socketDUDP, (struct sockaddr *)&selfAddressUDP, sizeof(selfAddressUDP)) < 0) {
    printf("Bind failed\n");
    close(socketDUDP);
    // fclose (fin);
    return 0;
  }

  //defining the remote address
  memset((char *) &remoteAddress, 0, sizeof(remoteAddress));
  remoteAddress.sin_family = AF_INET;
  remoteAddress.sin_port = htons(remotePortNum);
  if (inet_aton(remoteIP, &remoteAddress.sin_addr)==0) {
    fprintf(stderr, "inet_aton() failed\n");
    // fclose (fin);
    return(-1);
  }

  //Opening TCP/IP socket
  struct sockaddr_in selfAddressTCP;

  /*create a TCP/IP socket*/
  int socketDTCP;
  if((socketDTCP = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
      printf("Socket Creation Unsuccessful\n");
      // fclose (fin);
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
      // fclose (fin);
      return(-1);
    }
  selfAddressTCP.sin_port = htons(PORTNUMTCP);

  /*attempting to bind the socket descriptor to this IP address and port (myAddress)*/
  if(bind(socketDTCP, (struct sockaddr*)&selfAddressTCP, sizeof(selfAddressTCP)) < 0)
    {
      printf("Socket bind at user end unsuccessful\n");
      close(socketDTCP);
      // fclose (fin);
      return -1;
    }

  /*get self address information into myAddress
    unsigned int addrLength;
    addrLength = sizeof(myAddress);
    if(getsockname(socketD, (struct sockaddr*)&myAddress, &addrLength) < 0)
    {
    printf("Socket Information not obtained\n");
    close(socketD);
    return -1;
    }
    //Printing port number
    printf("Port Number : %d\n", ntohs(myAddress.sin_port));
  */

  /*setup the socket for listening upto a queue length of 10*/
  if(listen(socketDTCP, 10) < 0)
    {
      printf("Listen Failed");
      // fclose (fin);
      return (-1);
    }


  //now sending the message
  printf("Sending packet to %s port %d\n", remoteIP, remotePortNum);

  unsigned char buffer[BUFFERSIZE]; // messages are received in this buffer
  //send vs retrieve
  if(option == 0)
    {
      //MDFile(fileName);
      unsigned char MD5sum[16];
      char fullFileName[100];
      strcpy (fullFileName, directory);
      strcat (fullFileName, fileName);
      printf("check\n");
      printf("%s\n",fullFileName);
      
      int i = 0;
      
      int fileNameSize = getSize (fileName);

      // find the md5 sum and store in the MD5sum array
      MDFile(fullFileName, MD5sum);
      
      /* for (i = 0; i < fileNameSize; ++i) { */
      /* 	fputc(fileName[i], fin); */
      /* } */

      /* fputc('\t', fin); */

      /* for (i = 0; i < 16; ++i) { */
      /* 	fputc( MD5sum[i], fin); */
      /* } */

      /* fputc('\n', fin); */
      
      buffer[0] = '0';
      memcpy(buffer + 1, MD5sum, 16);
      memcpy(buffer + 17, &selfAddressTCP, sizeof(selfAddressTCP));
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
	  printf("Request Initiated\n");
	  int requestSocket = 0;
	  int addrLength = sizeof(remoteAddress);
	  while((requestSocket = accept(socketDTCP, (struct sockaddr *)&remoteAddress, &addrLength)) < 0)
            { 
	      /*Restart listening in the case of a special connection*/
	      if((errno != ECHILD) && (errno != ERESTART) && (errno != EINTR))
                {
		  printf("Accept Failed\n");
		  // fclose (fin);
		  return (-1);
                }
            }


	  printf("Connection Received from IP %s, Port# %d\n", inet_ntoa(remoteAddress.sin_addr), ntohs(remoteAddress.sin_port));
	  sendFile(requestSocket, fullFileName, getSize(fileName));
	  //shutdown initiated, not necessary
	  shutdown(requestSocket, 2);
	  return 0;
        }
    }

  else if(option == 1)
    {
      int i, size = getSize (fileName);
      if (size != 32) {
	printf("Invalid MD5sum\n");
	return -1;
      }
      
      unsigned char MD5sum[16];
      
      for (i = 0; i < 16; ++i) {
	MD5sum[i] = (fileName[2*i] - 48 - 39 * (fileName[2*i]/60))*16 + fileName[2*i+1] - 48 - 39 * (fileName[2*i+1]/60);
      }
      
      /* int ret = findMap (MD5sum, fileName); */
      /* if (ret == -1) { */
      /* 	printf("%s\n", fileName); */
      /* 	printf("file not found\n"); */
      /* 	fclose (fin); */
      /* 	return -1; */
      /* } */
      buffer[0] = '1';
      memcpy(buffer + 1, MD5sum, 16);
      memcpy(buffer + 17, &selfAddressTCP, sizeof(selfAddressTCP));
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
	  printf("Request Initiated\n");
	  int requestSocket = 0;
	  int addrLength = sizeof(remoteAddress);
	  while((requestSocket = accept(socketDTCP, (struct sockaddr *)&remoteAddress, &addrLength)) < 0)
            { 
	      /*Restart listening in the case of a special connection*/
	      if((errno != ECHILD) && (errno != ERESTART) && (errno != EINTR))
                {
		  printf("Accept Failed\n");
		  // fclose (fin);
		  return (-1);
                }
            }


	  printf("Connection Received from IP %s, Port# %d\n", inet_ntoa(remoteAddress.sin_addr), ntohs(remoteAddress.sin_port));
	  receiveFile(requestSocket, fileName, getSize(fileName), directory);
	  //shutdown initiated, not necessary
	  shutdown(requestSocket, 2);
	  // fclose (fin);
	  return (0);
        }
    }



  // fclose (fin);
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
  sprintf(fullFileName, "%s", fileName);
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


int receiveFile(int socketD, char* fileNameInput, int fileNameSize, char* dir)
{
  /*This variable indicates the number of bytes read*/

  int numBytes;
    
  /*Reading file name size*/
  int temp;
  numBytes = read(socketD, &temp, sizeof(int));
  printf("Filename size %d\n", temp);
  if (temp == -1) {
    printf("File not found\n");
    return -1;
  }
    
  /*Reading file name*/
  char fileName[temp + 1];
  numBytes = read(socketD, fileName, temp);
  fileName[temp] = 0;
  printf("Filename %s\n", fileName);

  /*Reading file size*/
  int fileSize;
    
  numBytes = read(socketD, &fileSize, sizeof(int));
  printf("Size of File %d bytes\n", fileSize);

  /*Writing to file*/
  FILE* outputFile;
  char fullFileName[50];
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
    }

  fclose(outputFile);
  printf("File Received\n");     
  return 0;
}

int getSize (char* str) 
{
  int count = 0;
  while (str[count++] != '\0') {
    
  }
  return count-1;
}

/* int findMap (unsigned char* MD5sum, char* fileName)  */
/* { */
/*   FILE *fin; */
/*   fin = fopen ("name-md5sum-map","r"); */
/*   while (!feof(fin)) { */
/*     char tempMD5sum[16]; */
/*     char tempFileName[100]; */
/*     int currentCount = 0; */
/*     int len; */
/*     char temp; */
/*     int status = 0; */
/*     while(1) */
/*       { */
/* 	temp = fgetc(fin); */
/* 	if(temp == EOF || temp == '\n') */
/* 	  break;   */
/* 	if (status == 1) { */
/* 	  tempMD5sum[currentCount - len] = temp; */
/* 	} */
	
/* 	if (temp == '\t') { */
/* 	  status = 1; */
/* 	  tempFileName[currentCount] = '\0'; */
/* 	  len = currentCount + 1; */
/* 	} */
/* 	if (status == 0) { */
/* 	  tempFileName[currentCount] = temp; */
/* 	} */
/* 	currentCount++; */
/*       } */
/*     if(temp == EOF) */
/*       break; */
    
/*     if (strcmp (tempFileName, fileName) == 0) { */
/*       memcpy (MD5sum, tempMD5sum, 16); */
/*       fclose (fin); */
/*       return 1; */
/*     } */
/*   } */
/*   fclose (fin); */
/*   return -1; */
/* } */
