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
	while (true)
	{
		int ready = ::poll(get().pollfd.data(), get().pollfd.size(), -1);
		if (get()._stop)
			return;
		if (ready == -1)
			throw std::runtime_error(std::string("Poll::poll(): ") + std::strerror(errno));
		get()._imtl();
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

void Poll::_imtl()
{
	Poll &poll = get();

	size_type size = poll.socket.size();
	for (size_type i = 0; i < size;)
	{
		if (poll.pollfd[i].revents & POLLIN)
			poll.socket[i]->pollin(poll.pollfd[i]);
		if (poll.pollfd[i].revents & POLLOUT)
			poll.socket[i]->pollout(poll.pollfd[i]);
		if (!(poll.pollfd[i].events & POLLIN) && !(poll.pollfd[i].events & POLLOUT))
		{
			remove(i);
			--size;
		}
		else
			++i;
	}
}

Poll &Poll::get()
{
	static Poll poll;
	return poll;
}
