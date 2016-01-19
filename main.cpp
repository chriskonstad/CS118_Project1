#include <iostream>

#include "server.h"

using std::endl;
using std::cout;
using std::runtime_error;

int main(int argc, char *argv[])
{
  try {
    Server server;
    server.run();
  } catch (runtime_error& e) {
    cout << e.what() << endl;
  }
}
