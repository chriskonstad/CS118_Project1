#pragma once

#include <netinet/in.h>
#include <string>
#include <sys/socket.h>

class Server {
 public:
  Server(int port = 5042);
  ~Server();

  void run();

 private:
  int mPort;
  int mSockfd;
  sockaddr_in mAddress;
  void error(const std::string& msg);
  void handleRequest(int socketfd, const sockaddr_in& cli_addr,
                     const socklen_t& cli_len);
};
