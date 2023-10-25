#include <set>
#include <vector>

#include "ListenSocket.hpp"
#include "Poll.hpp"

void addConnectionQueue(Poll &poll) {
  AConnection *connection = AConnection::popQueueFront();
  while (connection != NULL) {
    if (poll.add(connection)) {
      delete connection;
      std::cerr << "Failed to add connection to poll\n";
    }
    connection = AConnection::popQueueFront();
  }
}

int main(int, char **) {
  Poll poll;

  std::set<Address> listenAddresses = Address::resolveHost("localhost:8080");
  std::cout << "Resolved ListenSocket addresses: " << listenAddresses.size()
            << std::endl;

  std::set<Address>::iterator it = listenAddresses.begin();
  for (; it != listenAddresses.end(); it++) {
    ListenSocket *s = NULL;
    try {
      s = new ListenSocket(*it);
      if (poll.add(s))
        throw std::runtime_error("Failed to add listen socket to poll");
    } catch (const std::exception &e) {
      delete s;
      std::cout << e.what() << "\n";
      return 1;
    }
  }

  while (true) {
    std::cout << "New poll cycle" << std::endl;
    if (poll.update()) break;
    addConnectionQueue(poll);
  }
  return 0;
}
