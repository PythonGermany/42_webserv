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

AConnection::AConnection()
{
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
	}
	return *this;
}

void AConnection::send(std::string msg)
{
	_writeBuffer += msg;
}

void AConnection::pollout(struct pollfd &pollfd)
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
		throw std::runtime_error(std::string("AConnection::pollout(): ") + std::strerror(errno));
	_writeBuffer.erase(0, lenSent);
	if (_writeBuffer.empty())
		pollfd.events &= ~POLLOUT;
}

void AConnection::pollin(struct pollfd &pollfd)
{
	char tmpbuffer[BUFFER_SIZE];

	pollfd.revents &= ~POLLIN;
	ssize_t msglen = ::recv(pollfd.fd, tmpbuffer, BUFFER_SIZE, 0);
	if (msglen == -1)
		throw std::runtime_error(std::string("AConnection::pollin(): ") + std::strerror(errno));
	if (msglen == 0)
		pollfd.events &= ~POLLIN;
	_readBuffer += std::string(tmpbuffer, tmpbuffer + msglen);
	if (_readBuffer.size() > msgsizelimit)
	{
		pollfd.events = 0;
		pollfd.revents = 0;
		return;
	}
	passReadBuffer();
	if (!_writeBuffer.empty())
		pollfd.revents |= POLLOUT;
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
