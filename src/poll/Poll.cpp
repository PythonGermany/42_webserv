/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Poll.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jharrach <jharrach@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/21 16:39:41 by jharrach          #+#    #+#             */
/*   Updated: 2023/09/18 23:37:46 by jharrach         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Poll.hpp"

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

bool Poll::poll()
{
	Poll &poll = getInstance();
	static size_t numListenSockets = poll.callbackObjects.size();

	std::cout << "poll.timeout: " << poll.timeout << std::endl;
	int ready = ::poll(poll.pollfds.data(), poll.pollfds.size(), poll.timeout);
	if (poll.stop)
		return false;
	if (ready == -1)
		throw std::runtime_error(std::string("Poll::poll(): ") + std::strerror(errno));
	poll.timeout = -1;
	poll.iterate();
	if (poll.timeout == -1 && numListenSockets != poll.callbackObjects.size())
		poll.timeout = TIMEOUT;
	return true;
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

void Poll::cleanUp()
{
	Poll &poll = getInstance();

	std::vector<IFileDescriptor *>::iterator callbackObjectsIt = poll.callbackObjects.begin();
	std::vector<struct pollfd>::iterator pollfdIt = poll.pollfds.begin();
	while (callbackObjectsIt != poll.callbackObjects.end())
	{
		delete *callbackObjectsIt;
		close(pollfdIt->fd);
		++callbackObjectsIt;
		++pollfdIt;
	}
}

Poll::~Poll()
{
	cleanUp();
}

Poll &Poll::operator=(Poll const &)
{
	return *this;
}

void Poll::iterate()
{
	size_type size = callbackObjects.size();

	for (size_type i = 0; i < size;)
	{
		callbackObjects[i]->onPollEvent(pollfds[i]);
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

void Poll::addPollEvent(short event, IFileDescriptor *src)
{
	std::vector<IFileDescriptor *>::iterator callbackObjectsIt = getInstance().callbackObjects.begin();
	std::vector<struct pollfd>::iterator pollfdIt = getInstance().pollfds.begin();
	while (callbackObjectsIt != getInstance().callbackObjects.end())
	{
		if (*callbackObjectsIt == src)
		{
			pollfdIt->events |= event;
			return;
		}
		++callbackObjectsIt;
		++pollfdIt;
	}
	throw std::invalid_argument("Poll::addPollEvent");
}

void Poll::clearPollEvent(short event, IFileDescriptor *src)
{
	std::vector<IFileDescriptor *>::iterator callbackObjectsIt = getInstance().callbackObjects.begin();
	std::vector<struct pollfd>::iterator pollfdIt = getInstance().pollfds.begin();
	while (callbackObjectsIt != getInstance().callbackObjects.end())
	{
		if (*callbackObjectsIt == src)
		{
			pollfdIt->events &= ~event;
			return;
		}
		++callbackObjectsIt;
		++pollfdIt;
	}
	throw std::invalid_argument("Poll::addPollEvent");
}

void Poll::setPollActive(short oldEvents, IFileDescriptor *src)
{
	std::vector<IFileDescriptor *>::iterator callbackObjectsIt = getInstance().callbackObjects.begin();
	std::vector<struct pollfd>::iterator pollfdIt = getInstance().pollfds.begin();
	while (callbackObjectsIt != getInstance().callbackObjects.end())
	{
		if (*callbackObjectsIt == src)
		{
			pollfdIt->events &= ~POLLINACTIVE;
			pollfdIt->events |= oldEvents;
			return;
		}
		++callbackObjectsIt;
		++pollfdIt;
	}
	throw std::invalid_argument("Poll::addPollEvent");
}

short Poll::setPollInactive(IFileDescriptor *src)
{
	std::vector<IFileDescriptor *>::iterator callbackObjectsIt = getInstance().callbackObjects.begin();
	std::vector<struct pollfd>::iterator pollfdIt = getInstance().pollfds.begin();
	while (callbackObjectsIt != getInstance().callbackObjects.end())
	{
		if (*callbackObjectsIt == src)
		{
			short tmp = pollfdIt->events;
			pollfdIt->events &= ~POLLIN;
			pollfdIt->events |= POLLINACTIVE;
			return tmp;
		}
		++callbackObjectsIt;
		++pollfdIt;
	}
	throw std::invalid_argument("Poll::getPollEvent");
}

/**
 * only if an object exists does the destructor get called
*/
Poll &Poll::getInstance()
{
	static Poll poll;

	return poll;
}
