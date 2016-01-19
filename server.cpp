#include "server.h"

#include <stdio.h>
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <stdlib.h>
#include <strings.h>

#include <string>
#include <stdexcept>

#include <unistd.h>

using std::runtime_error;
using std::string;

Server::Server(int port) : mPort(port){
  mSockfd = socket(AF_INET, SOCK_STREAM, 0); //create socket
  if (mSockfd < 0) {
    error("ERROR opening socket");
  }
  memset((char *) &mAddress, 0, sizeof(mAddress));  //reset memory
  //fill in address info
  mAddress.sin_family = AF_INET;
  mAddress.sin_addr.s_addr = INADDR_ANY;
  mAddress.sin_port = htons(mPort);

  if (bind(mSockfd, (struct sockaddr *)&mAddress, sizeof(mAddress)) < 0) {
    error("ERROR on binding");
  }

  listen(mSockfd,5); //5 simultaneous connection at most
}

Server::~Server() {
  // Close the opened socket
  if(mSockfd >= 0) {
    close(mSockfd);
  }
}

void Server::run() {
  int newsockfd;
  socklen_t clilen;
  sockaddr_in serv_addr;
  sockaddr_in cli_addr;

  //accept connections
  while(true) {
    newsockfd = accept(mSockfd, (struct sockaddr *) &cli_addr, &clilen);
    handleRequest(newsockfd, cli_addr, clilen);
  }
}

void Server::handleRequest(int socketfd, const sockaddr_in &cli_addr,
                          const socklen_t &cli_len) {
  if (socketfd < 0) {
    error("ERROR on accept");
  }

  int n;
  char buffer[256];

  memset(buffer, 0, 256); //reset memory

  //read client's message
  n = read(socketfd,buffer,255);
  if (n < 0) error("ERROR reading from socket");
  printf("Here is the message: %s\n",buffer);

  //reply to client
  n = write(socketfd,"I got your message",18);
  if (n < 0) {
    error("ERROR writing to socket");
  }

  close(socketfd);//close connection 
}

void Server::error(const string& msg)
{
  //perror(msg);
  throw runtime_error(msg);
}
