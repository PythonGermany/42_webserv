/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AConnection.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jharrach <@student.42heilbronn.de>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/21 16:39:41 by jharrach          #+#    #+#             */
/*   Updated: 2023/09/08 22:28:53 by jharrach         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ACONNECTION_HPP
# define ACONNECTION_HPP

# define BUFFER_SIZE 1

# include "IFileDescriptor.hpp"
# include "Address.hpp"

# include <string>
# include <vector>

class AConnection : public IFileDescriptor
{
public:
	AConnection();
	AConnection(AConnection const &other);
	virtual ~AConnection();
	AConnection &operator=(AConnection const &other);

	virtual void OnHeadRecv(std::string msg) = 0;
	virtual void OnBodyRecv(std::string msg) = 0;
	virtual void OnCgiRecv(std::string msg) = 0;
	void send(std::string msg);
	void runCGI(std::string program, std::vector<std::string> &arg, std::vector<std::string> &env);
protected:
	Address client;
	Address host;
	std::string::size_type msgsizelimit;
	std::string::size_type msgsize;
	std::string	msgdelimiter;
private:
	std::string _writeBuffer;
	std::string _readBuffer;

	void pollout(struct pollfd &pollfd);
	void pollin(struct pollfd &pollfd);
	void passReadBuffer();
};

#endif //ACONNECTION_HPP
