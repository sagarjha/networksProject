#include <vector>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <list>

using namespace std;

// class for storing the mesh information
class Mesh {
  // an element i of nodes will store the IP address and port number of node i
  vector <sockaddr_in> nodes;
  // number of nodes in the mesh
  int numNodes;
  // extracts the portno, ipaddress and folder name from the string
  void extract (const string nodeInfo, int & portNo, string & IPAddress, string & folder) {
    // pos1 is the index of the colon, pos2 is the index of the space
    int pos1=0,pos2=0;
    // length of nodeInfo
    int len = nodeInfo.size();
    // find pos1 and pos2
    for (int i=0; i < len; ++i) {
      if (nodeInfo[i]==':') {
	pos1 = i;
      }
      else if (nodeInfo[i]==' ') {
	pos2 = i;
	break;
      }
    }
    // set the values as an appropriate substring of nodeInfo
    IPAddress = nodeInfo.substr (0, pos1);
    portNo = atoi(nodeInfo.substr (pos1+1, pos2-pos1).c_str());
    folder = nodeInfo.substr (pos2+1, len-pos2-1);
    return;
  }

 public:
  // the constructor for the class
  Mesh () {
    // open an input stream
    ifstream fin;
    // open the configuration file
    fin.open ("FileMesh.cfg");
    // read the file line by line and store in a list
    list <string> nodesList;
    // initialize numNodes to 0;
    numNodes = 0;
    // read to the end of the file
    while (!fin.eof ()) {
      string nodeInfo;
      getline(fin,nodeInfo,'\n');
      // increase the number of nodes by 1
      numNodes++;
      // push into the list
      nodesList.push_back(nodeInfo);
    }
    // resize the vector
    nodes.resize (numNodes);
    // populate the vector
    for (int i = 0; i < numNodes; ++i) {
      // take the front end of the list
      string nodeInfo = nodesList.front ();
      // port number of the node
      int portNo;
      // IP address and folder name for the node
      string IPAddress, folder;
      // extract portno, ipaddress and folder from the string
      extract(nodeInfo, portNo, IPAddress, folder);
      // pop it from the list
      nodesList.pop_front();
      // create a sockaddr_in structure
      struct sockaddr_in host;
      // set sin_family
      host.sin_family = AF_INET;
      // set port number
      host.sin_port = htons(portNo);
      // set IP address
      int ret = inet_aton(IPAddress.c_str(), &(host.sin_addr));
      // error checking
      if (ret == 0) {
	cout << "Invalid IP Address" << endl;
	cout << "Aborting !!" << endl;
	exit(0);
      }
      // zero the rest of the struct
      memset(&(host.sin_zero), '\0', 8);
      // store in the vector
      nodes[i] = host;
    }
  }

  sockaddr_in get (int i) {
    return nodes [i];
  }
};
