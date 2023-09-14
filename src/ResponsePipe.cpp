#include <iostream>

#include "../include/ResponsePipe.hpp"

ResponsePipe::ResponsePipe()
{
}

ResponsePipe::ResponsePipe(ResponsePipe const &other)
{
	*this = other;
}

ResponsePipe::~ResponsePipe()
{
}

ResponsePipe &ResponsePipe::operator=(ResponsePipe const &other)
{
	if (this != &other)
	{
	}
	return *this;
}
