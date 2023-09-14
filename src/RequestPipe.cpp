#include <iostream>

#include "../include/RequestPipe.hpp"

RequestPipe::RequestPipe()
{
}

RequestPipe::RequestPipe(AConnection *src, int pid)
{
	connection = src;
	cgiPid = pid;
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
		connection = other.connection;
		cgiPid = other.cgiPid;
	}
	return *this;
}

void RequestPipe::onPollOut(struct pollfd &pollfd)
{
	size_t lenToSend;
	ssize_t lenSent;

	pollfd.revents &= ~POLLOUT;
	if (_writeBuffer.size() > BUFFER_SIZE)
		lenToSend = BUFFER_SIZE;
	else
		lenToSend = _writeBuffer.size();
	lenSent = ::send(pollfd.fd, _writeBuffer.data(), lenToSend, 0);
	if (lenSent == -1)
		throw std::runtime_error(std::string("RequestPipe::onPollOut(): ") + std::strerror(errno));
	_writeBuffer.erase(0, lenSent);
	if (_writeBuffer.empty())
		pollfd.events &= ~POLLOUT;
}

void RequestPipe::onPollIn(struct pollfd &)
{
}

void RequestPipe::onNoPollIn()
