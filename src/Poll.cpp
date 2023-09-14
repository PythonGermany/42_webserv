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
	getInstance().callbackObjects.push_back(src);
}

void Poll::add(struct pollfd &src)
{
	getInstance().pollfds.push_back(src);
}

void Poll::remove(size_type pos)
{
	Poll &poll = getInstance();

	delete poll.callbackObjects[pos];
	close(poll.pollfds[pos].fd);
	poll.callbackObjects.erase(poll.callbackObjects.begin() + pos);
	poll.pollfds.erase(poll.pollfds.begin() + pos);
}

void Poll::poll()
{
	Poll &poll = getInstance();

	while (true)
	{
		std::cout << "poll: " << poll.timeout << std::endl;
		int ready = ::poll(poll.pollfds.data(), poll.pollfds.size(), poll.timeout);
		if (poll.stop)
			return;
		if (ready == -1)
			throw std::runtime_error(std::string("Poll::poll(): ") + std::strerror(errno));
		poll.iterate();
	}
}

/**
 * print newline because ^C is printed without
*/
void Poll::signalHandler(int)
{
	getInstance().stop = true;
	std::cout << std::endl;
}

Poll::Poll()
{
	timeout = -1;
	stop = false;
}

Poll::Poll(Poll const &)
{
}


Poll::~Poll()
{
	std::vector<IFileDescriptor *>::iterator callbackObjectsIt = callbackObjects.begin();
	std::vector<struct pollfd>::iterator pollfdIt = pollfds.begin();
	while (callbackObjectsIt != callbackObjects.end())
	{
		delete *callbackObjectsIt;
		close(pollfdIt->fd);
		++callbackObjectsIt;
		++pollfdIt;
	}
}

Poll &Poll::operator=(Poll const &)
{
	return *this;
}

// timeval operator-(timeval const &lhs, timeval const &rhs)
// {
// 	timeval result;
//     result.tv_sec = lhs.tv_sec - rhs.tv_sec;
//     result.tv_usec = lhs.tv_usec - rhs.tv_usec;

//     if (result.tv_usec < 0) {
//         result.tv_sec -= 1;
//         result.tv_usec += 1000000;
//     }
//     return result;
// }

// void Poll::_checkTimeout(struct pollfd &pollfd, AConnection *conn, int &timeout)
// {
// 	struct timeval currentTime;
// 	struct timeval delta;
// 	int newTimeout = TIMEOUT;

// 	gettimeofday(&currentTime, NULL);

// 	if (pollfd.revents & POLLIN || pollfd.revents & POLLOUT)
// 	{
// 		conn->lastTimeActive = currentTime;		
// 	}
// 	else
// 	{
// 		delta = currentTime - conn->lastTimeActive;
// 		newTimeout -= (delta.tv_sec * 1000 + delta.tv_usec / 1000);
// 	}
// 	if (newTimeout > 0 && (newTimeout < timeout || timeout == -1))
// 		timeout = newTimeout;
// 	if (newTimeout <= 0)
// 		pollfd.events = 0;
// }

void Poll::iterate()
{
	size_type size = callbackObjects.size();
	timeout = -1;

	for (size_type i = 0; i < size;)
	{
		if (pollfds[i].revents & POLLOUT)
			callbackObjects[i]->onPollOut(pollfds[i]);
		if (pollfds[i].revents & POLLIN)
			callbackObjects[i]->onPollIn(pollfds[i]);
		else
			callbackObjects[i]->onNoPollIn(pollfds[i]);
		if (pollfds[i].revents & (POLLERR | POLLNVAL))
			throw std::runtime_error("Poll::iterate(): revents was not handled");
		if (pollfds[i].events == 0)
		{
			remove(i);
			--size;
		}
		else
			++i;
	}
}

void Poll::setTimeout(int src)
{
	Poll &poll = getInstance();

	if (poll.timeout == -1 || src < poll.timeout)
		poll.timeout = src;
}

void Poll::setPollOut(IFileDescriptor *src)
{
	std::vector<IFileDescriptor *>::iterator callbackObjectsIt = getInstance().callbackObjects.begin();
	std::vector<struct pollfd>::iterator pollfdIt = getInstance().pollfds.begin();
	while (callbackObjectsIt != getInstance().callbackObjects.end())
	{
		if (*callbackObjectsIt == src)
		{
			pollfdIt->events |= POLLOUT;
			return;
		}
		++callbackObjectsIt;
		++pollfdIt;
	}
	throw std::invalid_argument("Poll::setPollOut");
}

/**
 * only if an object exists does the destructor get called
*/
Poll &Poll::getInstance()
{
	static Poll poll;

	return poll;
}
