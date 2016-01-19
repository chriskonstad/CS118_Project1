#include "server.h"

#include <stdio.h>
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <stdlib.h>
#include <strings.h>

#include <iostream>
#include <string>
#include <stdexcept>

#include <unistd.h>

using std::cout;
using std::endl;
using std::runtime_error;
using std::string;

Server::Buffer::Buffer(int size) : mSize(size) {
  mLocation = new char[size];
}

Server::Buffer::~Buffer() {
  delete[] mLocation;
}

char * Server::Buffer::data() {
  return mLocation;
}

int Server::Buffer::size() const {
  return mSize;
}

void Server::Buffer::zero() {
  memset(mLocation, 0, mSize);
}

Server::Server(int port) : mBuffer(mMaxBufferSize), mLog(cout), mPort(port) {
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
  mLog << "Initializing server on port " << mPort << endl;
}

Server::~Server() {
  mLog << "Stopping the server" << endl << "Closing port " << mPort << endl;
  // Close the opened socket
  close(mSockfd);
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

  int nBytesRead;
  int nBytesWritten;

  mBuffer.zero(); // reset memory

  //read client's message
  nBytesRead = read(socketfd, mBuffer.data() , mBuffer.size() - 1);
  if (nBytesRead < 0) {
    error("ERROR reading from socket");
  }
  mLog << "Received [" << nBytesRead << " bytes, allocated "
       << mMaxBufferSize << " bytes]:" << endl
       << mBuffer.data() << endl;

  //reply to client
  nBytesWritten = write(socketfd,"I got your message",18);
  if (nBytesWritten < 0) {
    error("ERROR writing to socket");
  }

  // TODO Not called if exception occurs above, FIX
  close(socketfd);  // close connection
}

void Server::error(const string& msg)
{
  throw runtime_error(msg);
}
