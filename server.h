#pragma once

#include <netinet/in.h>
#include <string>
#include <sys/socket.h>

// Handle HTTP requests for files
class Server {
 public:
  // Bind the server to the given port
  Server(int port = 5042);
  ~Server();

  // Start serving files
  void run();

  // Wrapper around char *
  class Buffer {
   public:
    Buffer(int size);
    ~Buffer();
    char* data();
    int size() const;
    void zero();

   private:
    char* mLocation;
    int mSize;
  };

 private:
  std::ostream& mLog;                  // logging utility
  int mPort;                           // port the server is bound to
  int mSockfd;                         // socketfd for accepting connections
  sockaddr_in mAddress;                // server's address
  void error(const std::string& msg);  // error function, THROWS
  void handleRequest(int socketfd, const sockaddr_in& cli_addr,
                     const socklen_t& cli_len);  // process a request
  static constexpr int mMaxBufferSize = 1024;    // max request size
  Buffer mBuffer;                                // buffer to hold request data
  std::string mServerPath;                       // path to the server
};
