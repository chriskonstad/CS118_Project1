#include "server.h"

#include <stdio.h>
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <stdlib.h>
#include <strings.h>

#include <ctime>
#include <iostream>
#include <locale>
#include <sstream>
#include <string>
#include <stdexcept>
#include <vector>

#include <unistd.h>

using std::cout;
using std::endl;
using std::localtime;
using std::runtime_error;
using std::string;
using std::stringstream;
using std::time_t;
using std::vector;

const string SERVER_NAME = "CS118 Project 1";

class Response {
  public:
    Response(string filepath);

    string createHeader() const;
    vector<char> data() const;
  private:
    enum class Status {
      OK = 200,
      NOT_MODIFIED = 304,
      BAD_REQUEST = 400,
      FORBIDDEN = 403,
      NOT_FOUND = 404,
      INT_SERV_ERR = 500,
    };
    enum class ContentType {
      HTML,
      GIF,
      JPEG,
    };
    string timeToString(time_t time) const;
    Status mStatus;
    ContentType mType;
    time_t mAccess;
    time_t mModified;
    vector<char> mData;
};

Response::Response(string filepath) {
  time(&mAccess);
  time(&mModified); // default value
  mStatus = Status::INT_SERV_ERR; // default
  mType = ContentType::HTML;  // default
}

vector<char> Response::data() const {
  return mData;
}

string Response::timeToString(time_t t) const {
  vector<char> dateStr;
  dateStr.resize(80);
  struct tm * timeinfo;
  timeinfo = localtime(&t);
  strftime(dateStr.data(), dateStr.size(), "%a, %e %b %Y %X", timeinfo);
  return string(dateStr.data());
}

string Response::createHeader() const {
  stringstream ss;
  ss << "HTTP/1.1 ";
  switch(mStatus) {
    case Status::OK:
      ss << "200 OK" << endl;
      break;
    case Status::NOT_MODIFIED:
      ss << "304 Not Modified" << endl;
      break;
    case Status::BAD_REQUEST:
      ss << "400 Bad Request" << endl;
      break;
    case Status::FORBIDDEN:
      ss << "403 Forbidden" << endl;
      break;
    case Status::NOT_FOUND:
      ss << "404 Not Found" << endl;
      break;
    case Status::INT_SERV_ERR:
      ss << "500 Internal Server Error" << endl;
      break;
  }

  ss << "Date: " << timeToString(mAccess) << endl;

  ss << "Server: " << SERVER_NAME << endl;

  ss << "Last-Modified: " << timeToString(mModified) << endl;

  ss << "Content-Type: ";
  switch(mType) {
    case ContentType::HTML:
      ss << "text/html" << endl;
      break;
    case ContentType::GIF:
      ss << "image/gif" << endl;
      break;
    case ContentType::JPEG:
      ss << "image/jpeg" << endl;
      break;
  }
  return ss.str();
}

string parseUri(Server::Buffer& buffer) {
  int start = 0;
  int end = 0;
  for(int i=0; i<buffer.size(); i++) {
    if('/' == buffer.data()[i]) {
      start = i;
      break;
    }
  }
  for(int i=start; i<buffer.size(); i++) {
    if(' ' == buffer.data()[i]) {
      end = i;
      break;
    }
  }
  return string(&buffer.data()[start], end - start);
}

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

Server::Server(int port)
    : mBuffer(mMaxBufferSize),
      mLog(cout),
      mPort(port) {
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

  // File to grab is between slash and a space
  mLog << "Parsed URI: " << parseUri(mBuffer) << endl;

  //reply to client
  Response response("foo.html");
  string header = response.createHeader();
  nBytesWritten = write(socketfd, header.c_str(), header.length());
  if (nBytesWritten < 0) {
    error("ERROR writing to socket");
  }
  mLog << "Replied [" << nBytesWritten << " bytes]:" << endl;
  mLog << header << endl << endl;

  // TODO Not called if exception occurs above, FIX
  close(socketfd);  // close connection
}

void Server::error(const string& msg)
{
  throw runtime_error(msg);
}

