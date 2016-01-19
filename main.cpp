#include <cstdlib>
#include <iostream>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdexcept>

#include "server.h"

using std::endl;
using std::cout;
using std::runtime_error;

Server *server;
void signal_handler(int s) {
  cout << "Closing server due to signal '" << strsignal(s) << "'" << endl;
  delete server;
  exit(1);
}

int main(int argc, char *argv[])
{
  struct sigaction handler;
  handler.sa_handler = signal_handler;
  sigemptyset(&handler.sa_mask);
  handler.sa_flags = 0;

  sigaction(SIGINT, &handler, NULL);
  sigaction(SIGKILL, &handler, NULL);
  sigaction(SIGTERM, &handler, NULL);

  try {
    server = new Server();
    server->run();
  } catch (runtime_error& e) {
    cout << e.what() << endl;
  }
  delete server;

  return 0;
}
