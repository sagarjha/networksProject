/** 
 * @file
 * 
 * @brief The configuration setup header file
 * 
 * @details This sets up the server configuration
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

/** struct for storing the mesh information
 * an element i of nodes will store the IP address and port number of node i
 */
struct sockaddr_in nodes[100];
// number of nodes in the mesh
int numNodes;

char folder[100][50];

/** extracts the portno, ipaddress and folder name from the string
 * @param nodeInfo a line extracted from the file
 * @param len length of line
 * @param portNo port number
 * @param IPAddress IP Address
 * @param folder folder name for storing the files
 */
void extract (char nodeInfo[500], int len, int* portNo, char IPAddress[50], char folder[50])
{
  // pos1 is the index of the colon, pos2 is the index of the space
  int pos1=0,pos2=0;
  // find pos1 and pos2
  int i;
  for (i=0; i < len; ++i) {
    if (nodeInfo[i]==':') {
      pos1 = i;
    }
    else if (nodeInfo[i]==' ') {
      pos2 = i;
      break;
    }
  }
  if(i == len)
    {
      printf("Invalid line format\n");
      exit(0);
    }
  // set the values as an appropriate substring of nodeInfo
  memcpy(IPAddress, nodeInfo, pos1);
  IPAddress[pos1] = 0;
  char portNoChar[10];
  memcpy(portNoChar, nodeInfo + pos1 + 1, pos2 - (pos1 + 1));
  portNoChar[pos2 - (pos1 + 1)] = 0;
  *portNo = atoi(portNoChar);
  memcpy(folder, nodeInfo + pos2+ 1, len-(pos2+1));
  folder[len - (pos2+1)] = 0;
  return;
}

/** generates the array of structures holding the individual node addresses
 */
void createMesh() {
  // open an input stream
  FILE* fin;
  // open the configuration file
  fin = fopen ("FileMesh.cfg", "r");
  if(fin == NULL)
    {
      printf("Cannot open FileMesh.cfg\n");
      exit(0);
    }
  // initialize numNodes to 0;
  numNodes = 0;
  // read to the end of the file
  while (!feof(fin)) {
    char nodeInfo[500];
    int currentCount = 0;
    char temp;
    while(1)
      {
	temp = fgetc(fin);
	if(temp == EOF || temp == '\n')
	  break;            
	nodeInfo[currentCount++] = temp;
      }
    if(temp == EOF)
      break;
    nodeInfo[currentCount] = 0;
    // increase the number of nodes by 1
    numNodes++;
    if(numNodes > 100)
      {
	printf("Too many nodes\n");
	exit(0);
      }
    // port number of the node
    int portNo;
    // IP address and folder name for the node
    char IPAddress[50];
    // extract portno, ipaddress and folder from the string
    extract(nodeInfo, currentCount, &portNo, IPAddress, folder[numNodes-1]);
    // create a sockaddr_in structure
    struct sockaddr_in host;
    // set sin_family
    host.sin_family = AF_INET;
    // set port number
    host.sin_port = htons(portNo);
    // set IP address
    int ret = inet_aton(IPAddress, &(host.sin_addr));
    // error checking
    if (ret == 0) {
      printf("Invalid IP Address\n");
      printf("Aborting !\n");
      exit(0);
    }
    // zero the rest of the struct
    memset(&(host.sin_zero), '\0', 8);
    // store in the vector
    nodes[numNodes - 1] = host;
  }
}

/** get the address of node i
 * @param i node id
 * @retval the structure containing the address
 */
struct sockaddr_in getAddress(int i) {
  return nodes[i];
}

/** get the folder where node i stores its files
 * @param i node id
 * @retval the folder name
 */
char* getFolder(int i) {
  return folder[i];
}
