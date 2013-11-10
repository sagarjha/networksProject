#include "config.h"
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char* argv[]) {
  
    if(argc != 2)
    {
        printf("Usage: <Executable> <node#>\n");
        return -1;
    }
    int selfId = atoi(argv[1]);
    Mesh M;


}
