#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include "mddriver.c"

int main(int argc, char* argv[])
{
  // n is the number of nodes
  int n = 7, i=0;
  // rem gives the md5sum remainder by number of nodes
  int rem = 0;
  if(argc != 3)
    {
      printf("Usage: <executable> <0 for send, 1 for retrieve> <filename>\n");
      return -1;
    }

  int option = atoi(argv[1]);
  char* fileName = argv[2];
  //MDFile(fileName);
  unsigned char MD5sum[16];
  // find the md5 sum and store in the MD5sum array
  MDFile(fileName, MD5sum);

  // each byte of the array is composed of two hexadecimal numbers
  for (i = 0; i < 16; i++) {
    int num = MD5sum[i];
    // extract the first hex digit and update the remainder
    rem = ((rem*16) + (num/16))%n;
    // do the same with the second hex digit
    rem = ((rem*16) + (num%16))%n;
  }
  printf("%d\n",rem);

  return 0;
}

