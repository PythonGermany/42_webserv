#include "../include/ListenSocket.hpp"
#include "../include/Poll.hpp"
#include <signal.h>

int main()
{
    signal(SIGINT, Poll::signalHandler);    
    Poll::add(new ListenSocket("0.0.0.0", "8080"));
    Poll::poll();
}
