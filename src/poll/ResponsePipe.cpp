#include <iostream>

#include "ResponsePipe.hpp"
#include "Poll.hpp"

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

ResponsePipe::ResponsePipe()
{
	gettimeofday(&lastTimeActive, NULL);
}

ResponsePipe::ResponsePipe(AConnection *callbackObject, int cgiPid) : _callbackObject(callbackObject), _cgiPid(cgiPid)
{
	gettimeofday(&lastTimeActive, NULL);
	_callbackObjectFlags = Poll::setPollInactive(_callbackObject);
}

ResponsePipe::ResponsePipe(ResponsePipe const &other)
{
	*this = other;
}

ResponsePipe::~ResponsePipe()
{
	Poll::setPollActive(_callbackObjectFlags, _callbackObject);
}

ResponsePipe &ResponsePipe::operator=(ResponsePipe const &other)
{
	if (this != &other)
	{
		_callbackObject = other._callbackObject;
		_readBuffer = other._readBuffer;
	}
	return *this;
}

void ResponsePipe::onPollEvent(struct pollfd &pollfd)
{
	char tmpbuffer[BUFFER_SIZE];

	if ((pollfd.revents & (POLLIN | POLLHUP)) == false)
	{
		onNoPollEvent(pollfd);
		return;
	}
	pollfd.revents &= ~(POLLIN | POLLHUP);
	ssize_t msglen = ::read(pollfd.fd, tmpbuffer, BUFFER_SIZE);
	if (msglen == -1)
		throw std::runtime_error(std::string("ResponsePipe::onPollEvent(): ") + std::strerror(errno));
	if (msglen == 0)
	{
		pollfd.events &= ~POLLIN;
		_callbackObject->OnCgiRecv(_readBuffer);
		int status;
		int ret = waitpid(_cgiPid, &status, 0);
		if (ret == -1)
			throw std::runtime_error(std::string("ResponsePipe::onPollEvent(): ") + std::strerror(errno));
		if (ret == 0)
			throw std::runtime_error("ResponsePipe::onPollEvent(): waitpid wasn't ready");//if WNOHANG
		std::cout << "cgiPid status: " << status << std::endl;
		return;
	}
	_readBuffer += std::string(tmpbuffer, tmpbuffer + msglen);
	gettimeofday(&lastTimeActive, NULL);
	//TODO max cgi message size
}

void ResponsePipe::onNoPollEvent(struct pollfd &pollfd)
{
	struct timeval currentTime;
	struct timeval delta;
	int timeout;

	gettimeofday(&currentTime, NULL);
	delta = currentTime - lastTimeActive;
	timeout =  TIMEOUT - (delta.tv_sec * 1000 + delta.tv_usec / 1000);
	if (timeout <= 0)
	{
		pollfd.events = 0;
		kill(_cgiPid, SIGKILL);
		int status;
		int ret = waitpid(_cgiPid, &status, 0);
		if (ret == -1)
			throw std::runtime_error(std::string("ResponsePipe::onPollEvent(): ") + std::strerror(errno));
		if (ret == 0)
			throw std::runtime_error("ResponsePipe::onPollEvent(): waitpid wasn't ready");//if WNOHANG
		std::cout << "cgiPid status: " << status << std::endl;
		_callbackObject->OnCgiTimeout();
		return;
	}
	std::cout << "new timeout: " << timeout << std::endl;
	Poll::setTimeout(timeout);
}
