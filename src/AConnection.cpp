/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AConnection.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jharrach <@student.42heilbronn.de>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/21 16:39:41 by jharrach          #+#    #+#             */
/*   Updated: 2023/09/08 22:26:21 by jharrach         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>

#include "../include/AConnection.hpp"
// #include "../include/CommonGatewayInterface.hpp"
#include "../include/Poll.hpp"

#include <unistd.h>
#include <fcntl.h>

AConnection::AConnection()
{
	gettimeofday(&lastTimeActive, NULL);
	Poll::setTimeout(TIMEOUT);
}

AConnection::AConnection(AConnection const &other)
{
	*this = other;
}


AConnection::~AConnection()
{
}

AConnection &AConnection::operator=(AConnection const &other)
{
	if (this != &other)
	{
		client = other.client;
		host = other.host;
		msgsizelimit = other.msgsizelimit;
		msgsize = other.msgsize;
		msgdelimiter = other.msgdelimiter;
		_writeBuffer = other._writeBuffer;
		_readBuffer = other._readBuffer;
		lastTimeActive = other.lastTimeActive;
	}
	return *this;
}

void AConnection::send(std::string msg)
{
	_writeBuffer += msg;
	Poll::setPollOut(this);
}

// void AConnection::runCGI(std::string program, std::vector<std::string> &arg, std::vector<std::string> &env)
// {
// 	int pid;
// 	int pipefd[2];

// 	if (pipe(pipefd) == -1)
// 		throw std::runtime_error(std::string("AConnection::runCGI(): ") + std::strerror(errno));
// 	pid = fork();
// 	if (pid == -1)
// 		throw std::runtime_error(std::string("AConnection::runCGI(): ") + std::strerror(errno));
// 	if (pid == 0)
// 	{
// 		if (dup2(pipefd[1], STDOUT_FILENO) == -1)
// 		{
// 			std::cerr << "webserv: error: dup2(): " << std::strerror(errno) << std::endl;
// 			exit(EXIT_FAILURE);
// 		}
// 		close(pipefd[0]);
// 		close(pipefd[1]);
// 		std::vector<char *> c_arg;
// 		std::vector<char *> c_env;
// 		c_arg.push_back(const_cast<char *>(program.c_str()));
// 		for (std::vector<std::string>::iterator it = arg.begin(); it != arg.end(); ++it)
// 			c_arg.push_back(const_cast<char *>(it->c_str()));
// 		c_arg.push_back(NULL);
// 		for (std::vector<std::string>::iterator it = env.begin(); it != env.end(); ++it)
// 			c_env.push_back(const_cast<char *>(it->c_str()));
// 		c_env.push_back(NULL);
// 		execve(program.c_str(), c_arg.data(), c_env.data());
// 		std::cerr << "webserv: error: execve(): " << std::strerror(errno) << std::endl;
// 		exit(EXIT_FAILURE);
// 	}
// 	Poll::add(new CommonGatewayInterface(*this));
// 	struct pollfd pollfd;

// 	pollfd.events = POLLIN;
// 	pollfd.fd = pipefd[0];
// 	close(pipefd[1]);
// 	std::cout << "fd: " << pollfd.fd << std::endl;
// 	pollfd.revents = 0;
// 	Poll::add(pollfd);
// }

void AConnection::onPollOut(struct pollfd &pollfd)
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
		throw std::runtime_error(std::string("AConnection::onPollOut(): ") + std::strerror(errno));
	_writeBuffer.erase(0, lenSent);
	if (_writeBuffer.empty())
		pollfd.events &= ~POLLOUT;
}

void AConnection::onPollIn(struct pollfd &pollfd)
{
	char tmpbuffer[BUFFER_SIZE];

	gettimeofday(&lastTimeActive, NULL);
	pollfd.revents &= ~POLLIN;
	ssize_t msglen = ::recv(pollfd.fd, tmpbuffer, BUFFER_SIZE, 0);
	if (msglen == -1)
		throw std::runtime_error(std::string("AConnection::onPollIn(): ") + std::strerror(errno));
	if (msglen == 0)
	{
		pollfd.events &= ~POLLIN;
		return;
	}
	Poll::setTimeout(TIMEOUT);
	_readBuffer += std::string(tmpbuffer, tmpbuffer + msglen);
	if (_readBuffer.size() > msgsizelimit)
	{
		pollfd.events = 0;
		pollfd.revents = 0;
		return;
	}
	passReadBuffer();
	// if (!_writeBuffer.empty())
	// 	pollfd.events |= POLLOUT;
}

timeval operator-(timeval const &lhs, timeval const &rhs)
{
	timeval result;
    result.tv_sec = lhs.tv_sec - rhs.tv_sec;
    result.tv_usec = lhs.tv_usec - rhs.tv_usec;

    if (result.tv_usec < 0) {
        result.tv_sec -= 1;
        result.tv_usec += 1000000;
    }
    return result;
}

void AConnection::onNoPollIn(struct pollfd &pollfd)
{
	struct timeval currentTime;
	struct timeval delta;
	int timeout;

	if (pollfd.events & POLLOUT)
		return;
	gettimeofday(&currentTime, NULL);
	delta = currentTime - lastTimeActive;
	timeout =  TIMEOUT - (delta.tv_sec * 1000 + delta.tv_usec / 1000);
	if (timeout <= 0)
	{
		pollfd.events = 0;
		return;
	}
	Poll::setTimeout(timeout);
}

void AConnection::passReadBuffer()
{
	std::string::size_type pos;
	while (true)
	{
		pos = _readBuffer.find(msgdelimiter);
		if (pos != std::string::npos)
		{
			pos += msgdelimiter.size();
			OnHeadRecv(_readBuffer.substr(0, pos));
			_readBuffer.erase(0, pos);
			continue;
		}
		if (_readBuffer.size() > msgsize)
		{
			OnBodyRecv(_readBuffer.substr(0, msgsize));
			_readBuffer.erase(0, msgsize);
			continue;
		}
		break;
	}
}
