#pragma once

#include <sys/socket.h>
#include <netinet/in.h>

class Server {
 public:
  Server(int port = 5042);
  ~Server();

  void run();

 private:
  int mPort;
  int mSockfd;
  sockaddr_in mAddress;
  void error(char* msg);
  void handleRequest(int socketfd, const sockaddr_in& cli_addr,
                     const socklen_t& cli_len);
};
