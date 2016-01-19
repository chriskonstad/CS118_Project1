#pragma once

#include <netinet/in.h>
#include <string>
#include <sys/socket.h>

class Server {
 public:
  Server(int port = 5042);
  ~Server();

  void run();

  class Buffer {
    public:
      Buffer(int size);
      ~Buffer();
      char* data();
      int size() const;
      void zero();
    private:
      char *mLocation;
      int mSize;
  };
 private:
  std::ostream& mLog;
  int mPort;
  int mSockfd;
  sockaddr_in mAddress;
  void error(const std::string& msg);
  void handleRequest(int socketfd, const sockaddr_in& cli_addr,
                     const socklen_t& cli_len);
  static constexpr int mMaxBufferSize = 1024;
  Buffer mBuffer;
};
