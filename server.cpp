#include "server.h"

#include <cerrno>
#include <chrono>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <locale>
// constants and structures needed for internet domain addresses,
// e.g. sockaddr_in
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <stdexcept>
// definitions of structures needed for sockets, e.g. sockaddr
#include <sys/socket.h>
#include <sys/stat.h>
// definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/types.h>
#include <unistd.h>
#include <vector>

using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::chrono::system_clock;
using std::chrono::time_point;
using std::cout;
using std::endl;
using std::ifstream;
using std::localtime;
using std::runtime_error;
using std::string;
using std::stringstream;
using std::time_t;
using std::vector;

const string SERVER_NAME = "CS118 Project 1";
const string FILE_403 = "./static/403.html";
const string FILE_404 = "./static/404.html";

// Wraps a char * up in a RAII wrapper object so that it is always freed
class SysCallGuard {
 public:
  SysCallGuard(char* data) : mData(data) {}
  ~SysCallGuard() {
    if (mData) {
      free(mData);
    }
  }

  char* data() const { return mData; }

 private:
  char* mData;
};

// Class for representing a response
// It is used to hold data for the header and to read files
class Response {
 public:
  // Read the file at filepath and build the response
  Response(const string& serverPath, string filepath);

  // Create the HTTP header
  string createHeader() const;

  // Get the response's data
  vector<char> data() const;

 private:
  enum class Status {
    OK = 200,
    FORBIDDEN = 403,
    NOT_FOUND = 404,
    INT_SERV_ERR = 500,
  };
  enum class ContentType {
    HTML,
    TEXT,
    GIF,
    JPEG,
  };
  // Turn a time_t to the string format needed for HTTP
  string timeToString(time_t time) const;
  // Load the given file
  void loadFile(const string& filepath);

  // HTTP header data
  Status mStatus;
  ContentType mType;
  time_t mAccess;
  time_t mModified;
  vector<char> mData;
};

void Response::loadFile(const string& filepath) {
  // Get the file extension
  int pos = filepath.rfind(".");
  string ext = filepath.substr(pos + 1);
  if ("html" == ext) {
    mType = ContentType::HTML;
  } else if ("txt" == ext) {
    mType = ContentType::TEXT;
  } else if ("jpeg" == ext || "jpg" == ext) {
    mType = ContentType::JPEG;
  } else if ("gif" == ext) {
    mType = ContentType::GIF;
  } else {
    mType = ContentType::HTML;
    mStatus = Status::FORBIDDEN;
    mData.clear();
    loadFile(FILE_403);
    return;
  }

  // Load the file, or an error page if an error occurred
  ifstream file(filepath, std::ios::binary | std::ios::ate);
  if (!file && FILE_404 != filepath) {
    mStatus = Status::NOT_FOUND;
    loadFile(FILE_404);
    return;
  } else if (!file && FILE_404 == filepath) {
    // There was an error loading the 404 file, so show an internal server error
    mStatus = Status::INT_SERV_ERR;
    return;
  } else {
    // mstatus defaults to 200 OK

    // Get file's last modified time
    struct stat attributes;
    stat(filepath.c_str(), &attributes);
    mModified = attributes.st_mtime;

    // Get the length of the file
    file.seekg(0, file.end);
    int filesize = file.tellg();
    file.seekg(0, file.beg);

    // Read the file into the data vector
    mData.resize(filesize);
    file.read(mData.data(), mData.size());
  }
}

bool isInFileTree(const string& tree, const char* path) {
  size_t pathLength = strlen(path);

  // If path is in tree, it must be at least as long as tree
  if (pathLength < tree.size()) {
    return false;
  }

  // Compare path to tree
  for (size_t i = 0; i < tree.size(); i++) {
    if (tree[i] != path[i]) {
      return false;
    }
  }

  return true;
}

// Create a response for the file at filepath
Response::Response(const string& serverPath, string filepath) {
  filepath.insert(0, ".");
  time(&mAccess);
  time(&mModified);           // default value
  mStatus = Status::OK;       // default, let's be optimistic
  mType = ContentType::HTML;  // default

  // Ensure file is in server's tree
  SysCallGuard realFilePath(realpath(filepath.data(), nullptr));
  if (!realFilePath.data()) {
    // This will load either 403 or 404 depending on requested file
    loadFile(filepath);
  } else if (isInFileTree(serverPath, realFilePath.data())) {
    loadFile(filepath);
  } else {
    loadFile(FILE_403);
  }
}

vector<char> Response::data() const { return mData; }

string Response::timeToString(time_t t) const {
  vector<char> dateStr;
  dateStr.resize(80);
  struct tm* timeinfo;
  timeinfo = localtime(&t);
  strftime(dateStr.data(), dateStr.size(), "%a, %e %b %Y %X", timeinfo);
  return string(dateStr.data());
}

string Response::createHeader() const {
  stringstream ss;
  ss << "HTTP/1.1 ";
  switch (mStatus) {
    case Status::OK:
      ss << "200 OK" << endl;
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
  switch (mType) {
    case ContentType::HTML:
      ss << "text/html" << endl;
      break;
    case ContentType::TEXT:
      ss << "text/plain" << endl;
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

// Get the requested file from an HTTP request
string parseUri(Server::Buffer& buffer) {
  int start = 0;
  int end = 0;
  for (int i = 0; i < buffer.size(); i++) {
    if ('/' == buffer.data()[i]) {
      start = i;
      break;
    }
  }
  for (int i = start; i < buffer.size(); i++) {
    if (' ' == buffer.data()[i]) {
      end = i;
      break;
    }
  }
  return string(&buffer.data()[start], end - start);
}

// Wraps the socketfd up in a RAII object so that sockets are always closed,
// even if exceptions are called
class SocketGuard {
 public:
  SocketGuard(int socketfd) : mSocketFd(socketfd) {}
  ~SocketGuard() { close(mSocketFd); }

  int socket() const { return mSocketFd; }

 private:
  int mSocketFd;
};

Server::Buffer::Buffer(int size) : mSize(size) { mLocation = new char[size]; }

Server::Buffer::~Buffer() { delete[] mLocation; }

char* Server::Buffer::data() { return mLocation; }

int Server::Buffer::size() const { return mSize; }

void Server::Buffer::zero() { memset(mLocation, 0, mSize); }

Server::Server(int port) : mLog(cout), mPort(port), mBuffer(mMaxBufferSize) {
  mSockfd = socket(AF_INET, SOCK_STREAM, 0);  // create socket
  if (mSockfd < 0) {
    error("ERROR opening socket");
  }
  memset((char*)&mAddress, 0, sizeof(mAddress));  // reset memory

  // fill in address info
  mAddress.sin_family = AF_INET;
  mAddress.sin_addr.s_addr = INADDR_ANY;
  mAddress.sin_port = htons(mPort);

  if (bind(mSockfd, (struct sockaddr*)&mAddress, sizeof(mAddress)) < 0) {
    error("ERROR on binding");
  }

  // Store the server's path
  SysCallGuard serverPath(realpath(".", nullptr));
  mServerPath = string(serverPath.data());
  mLog << "Server's tree: " << mServerPath << endl;

  listen(mSockfd, 5);  // 5 simultaneous connection at most
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
  sockaddr_in cli_addr;

  // accept connections
  while (true) {
    newsockfd = accept(mSockfd, (struct sockaddr*)&cli_addr, &clilen);
    handleRequest(newsockfd, cli_addr, clilen);
  }
}

void Server::handleRequest(int socketfd, const sockaddr_in& cli_addr,
                           const socklen_t& cli_len) {
  // Use system_clock because monotonic_click is deprecated
  // and steady_clock is not supported on GCC 4.6.1.
  // Yes, system_clock isn't ideal for this but it works well enough
  time_point<system_clock> start, end;
  time_point<system_clock> startRead, endRead;
  time_point<system_clock> startWrite, endWrite;
  time_point<system_clock> startFile, endFile;
  start = system_clock::now();
  if (socketfd < 0) {
    error("ERROR on accept");
  }
  SocketGuard socketGuard(socketfd);

  int nBytesRead;
  int totalBytesWritten = 0;
  vector<int> nBytesWritten;
  nBytesWritten.reserve(3);

  mBuffer.zero();  // reset memory

  // read client's message
  startRead = system_clock::now();
  nBytesRead = read(socketGuard.socket(), mBuffer.data(), mBuffer.size() - 1);
  if (nBytesRead < 0) {
    error("ERROR reading from socket");
  }
  endRead = system_clock::now();
  mLog << "Received [" << nBytesRead << " bytes, allocated " << mMaxBufferSize
       << " bytes]:" << endl
       << mBuffer.data() << endl;

  // reply to client
  startFile = system_clock::now();
  Response response(mServerPath, parseUri(mBuffer));
  endFile = system_clock::now();

  string header = response.createHeader();
  startWrite = system_clock::now();
  nBytesWritten.push_back(
      write(socketGuard.socket(), header.data(), header.size()));
  nBytesWritten.push_back(write(socketGuard.socket(), "\n", 1));
  nBytesWritten.push_back(write(socketGuard.socket(), response.data().data(),
                                response.data().size()));
  for (auto& i : nBytesWritten) {
    if (i < 0) {
      error("ERROR writing to socket");
    }
    totalBytesWritten += i;
  }
  endWrite = system_clock::now();
  mLog << "Replied [" << totalBytesWritten << " bytes]:" << endl;
  mLog << header << endl;

  end = system_clock::now();
  auto elapsed = duration_cast<microseconds>(end - start).count();
  auto elapsedRead = duration_cast<microseconds>(endRead - startRead).count();
  auto elapsedWrite =
      duration_cast<microseconds>(endWrite - startWrite).count();
  auto elapsedFile = duration_cast<microseconds>(endFile - start).count();
  mLog << "[Processing (total): " << elapsed << " microseconds]" << endl
       << "[Read: " << elapsedRead << " microseconds]" << endl
       << "[File: " << elapsedFile << " microseconds]" << endl
       << "[Write: " << elapsedWrite << " microseconds]" << endl
       << endl;
}

void Server::error(const string& msg) {
  string message = msg;
  if (errno) {
    message += " (errno: " + string(strerror(errno)) + ")";
  }
  throw runtime_error(message);
}
