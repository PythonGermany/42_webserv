#include "../include/ListenSocket.hpp"
#include "../include/Pollstructs.hpp"

int main()
{
    Pollstructs data;
    data.add(ListenSocket("127.0.0.1", "8080"));
    data.add(ListenSocket("::1", "8080"));
    data.add(ListenSocket("127.0.0.1", "9000"));
    data.poll();
}
