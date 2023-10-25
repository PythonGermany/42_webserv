#include <set>
#include <vector>

#include "ListenSocket.hpp"
#include "Poll.hpp"

int addListenSockets(Poll &poll) {
  std::set<Address> listenAddresses = Address::resolveHost("localhost:8080");

  std::set<Address>::iterator it = listenAddresses.begin();
  for (; it != listenAddresses.end(); it++) {
    ListenSocket *s = NULL;
    try {
      s = new ListenSocket(*it);
      if (poll.add(s)) throw std::exception();
    } catch (...) {
      delete s;
      return 1;
    }
  }
  return 0;
}

int main(int, char **) {
  Poll poll;

  if (addListenSockets(poll)) {
    std::cerr << "Failed to add listen sockets to poll\n";
    return 1;
  }

  size_t cycleId = 0;
  while (true) {
    std::cout << "Poll cycle " << cycleId++ << std::endl;
    if (poll.update()) break;
  }
  return 0;
}
