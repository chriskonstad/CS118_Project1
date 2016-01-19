/* A simple server in the internet domain using TCP
   The port number is passed as an argument 
   This version runs forever, forking off a separate 
   process for each connection
   */
#include <stdio.h>

#include "server.h"

int main(int argc, char *argv[])
{
  Server server;
  server.run();
}
