/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   template.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jharrach <jharrach@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/21 16:39:41 by jharrach          #+#    #+#             */
/*   Updated: 2023/05/21 21:38:49 by jharrach         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>

#include "../include/CommonGatewayInterface.hpp"
#include <unistd.h>

CommonGatewayInterface::CommonGatewayInterface(AConnection &Connection) : _conn(Connection)
{
}

CommonGatewayInterface::CommonGatewayInterface(CommonGatewayInterface const &other) : _conn(other._conn), _readBuffer(other._readBuffer)
{
}

CommonGatewayInterface::~CommonGatewayInterface()
{
}

CommonGatewayInterface &CommonGatewayInterface::operator=(CommonGatewayInterface const &other)
{
	if (this != &other)
	{
		_conn = other._conn;
		_readBuffer = other._readBuffer;
	}
	return *this;
}

void CommonGatewayInterface::send(std::string msg)
{
	std::cout << "CGI::send(): " << msg << std::endl;
	_conn.OnCgiRecv("You send me something\n");
}

void CommonGatewayInterface::pollout(struct pollfd &)
{
}

void CommonGatewayInterface::pollin(struct pollfd &pollfd)
{
	char tmpbuffer[BUFFER_SIZE];

	pollfd.revents &= ~POLLIN;
	ssize_t msglen = ::read(pollfd.fd, tmpbuffer, BUFFER_SIZE);
	if (msglen == -1)
		throw std::runtime_error(std::string("CommonGatewayInterface::pollin(): ") + std::strerror(errno));
	if (msglen == 0)
	{
		pollfd.events = 0;
		pollfd.revents = 0;
		_conn.OnCgiRecv(_readBuffer);
		return;
	}
	_readBuffer += std::string(tmpbuffer, tmpbuffer + msglen);
}
