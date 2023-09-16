#include <iostream>

#include "../include/RequestPipe.hpp"

RequestPipe::RequestPipe()
{
}

RequestPipe::RequestPipe(AConnection *src)
{
	callbackObject = src;
}

RequestPipe::RequestPipe(RequestPipe const &other)
{
	*this = other;
}

RequestPipe::~RequestPipe()
{
}

RequestPipe &RequestPipe::operator=(RequestPipe const &other)
{
	if (this != &other)
	{
		callbackObject = other.callbackObject;
	}
	return *this;
}

void RequestPipe::onPollEvent(struct pollfd &pollfd)
{
	callbackObject->onPipePollOut(pollfd);
}
