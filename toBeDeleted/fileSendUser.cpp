/** @file fileSendUser.cpp
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
#define PORTNUM 3333


/**
 * The function to setup the connection between the user and the server
 */
int setupConnection(char *server, int portNum);

int main(int argc, char* argv[])
{
    /*The socket descriptor for the tcp connection*/
    int fd; 

    /*creating the socket*/
    if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Socket Creation Unsuccessul, Exiting... \n");
        return -1;
    }

    

}

int setupConnection(char *server, int portNum)
{
    printf("Server %s Port# d \n", server, portNum);
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
    myAddress.sin_port = htons(PORTNUM);

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
    if (inet_aton(serverAddress, &remoteAddress.sin_addr)==0) 
    {
        printf("Remote address allocation failed\n");
        return(-1);
    }
    remoteAddress.sin_port = htons(PORTNUM);

    /*Trying to establish connection*/
    if(connect(socketD, (struct sockaddr*)&remoteAddress, sizeof(remoteAddress)) < 0)
    {
        printf("Connection to server failed\n");
        return -1;
    }

    return 0;
}



   


