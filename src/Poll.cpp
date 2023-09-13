/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Poll.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jharrach <@student.42heilbronn.de>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/21 16:39:41 by jharrach          #+#    #+#             */
/*   Updated: 2023/09/08 23:13:54 by jharrach         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Poll.hpp"

#include <unistd.h>
#include <stdexcept>
#include <cstring>
#include <cerrno>
#include <iostream>

void Poll::add(IFileDescriptor *src)
{
	get().socket.push_back(src);
}

void Poll::add(struct pollfd &src)
{
	get().pollfd.push_back(src);
}

void Poll::remove(size_type pos)
{
	Poll &poll = get();
	
	delete poll.socket[pos];
	close(poll.pollfd[pos].fd);
	poll.socket.erase(poll.socket.begin() + pos);
	poll.pollfd.erase(poll.pollfd.begin() + pos);
}

void Poll::poll()
{
	int timeout = -1;

	while (true)
	{
		std::cout << "poll: " << timeout << std::endl;
		int ready = ::poll(get().pollfd.data(), get().pollfd.size(), timeout);
		if (get()._stop)
			return;
		if (ready == -1)
			throw std::runtime_error(std::string("Poll::poll(): ") + std::strerror(errno));
		timeout = get()._imtl();
	}
}

/**
 * print newline because ^C is printed without
*/
void Poll::signalHandler(int)
{
	get()._stop = true;
	std::cout << std::endl;
}

Poll::Poll() : _stop(false)
{
}

Poll::Poll(Poll const &)
{
}


Poll::~Poll()
{
	std::vector<IFileDescriptor *>::iterator socketIt = socket.begin();
	std::vector<struct pollfd>::iterator pollfdIt = pollfd.begin();
	while (socketIt != socket.end())
	{
		delete *socketIt;
		close(pollfdIt->fd);
		++socketIt;
		++pollfdIt;
	}
}

Poll &Poll::operator=(Poll const &)
{
	return *this;
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

void Poll::_checkTimeout(struct pollfd &pollfd, AConnection *conn, int &timeout)
{
	struct timeval currentTime;
	struct timeval delta;
	int newTimeout = TIMEOUT;

	gettimeofday(&currentTime, NULL);

	if (pollfd.revents & POLLIN || pollfd.revents & POLLOUT)
	{
		conn->lastTimeActive = currentTime;		
	}
	else
	{
		delta = currentTime - conn->lastTimeActive;
		newTimeout -= (delta.tv_sec * 1000 + delta.tv_usec / 1000);
	}
	if (newTimeout > 0 && (newTimeout < timeout || timeout == -1))
		timeout = newTimeout;
	if (newTimeout <= 0)
		pollfd.events = 0;
}

int Poll::_imtl()
{
	Poll &poll = get();
	size_type size = poll.socket.size();
	int timeout = -1;

	for (size_type i = 0; i < size;)
	{
		AConnection *conn = dynamic_cast<AConnection *>(poll.socket[i]);
		if (conn != NULL)
			_checkTimeout(poll.pollfd[i], conn, timeout);
		if (poll.pollfd[i].revents & POLLIN)
			poll.socket[i]->pollin(poll.pollfd[i]);
		if (poll.pollfd[i].revents & POLLOUT)
			poll.socket[i]->pollout(poll.pollfd[i]);
		if (!(poll.pollfd[i].revents & POLLOUT || poll.pollfd[i].revents & POLLIN) && poll.pollfd[i].revents)
			throw std::runtime_error("revents");
		if (!(poll.pollfd[i].events & POLLIN) && !(poll.pollfd[i].events & POLLOUT))
		{
			remove(i);
			--size;
		}
		else
			++i;
	}
	return timeout;
}

Poll &Poll::get()
{
	static Poll poll;
	return poll;
}
