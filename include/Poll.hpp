/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Poll.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jharrach <@student.42heilbronn.de>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/21 16:39:41 by jharrach          #+#    #+#             */
/*   Updated: 2023/09/08 22:35:44 by jharrach         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef POLL_HPP
# define POLL_HPP

# include "IFileDescriptor.hpp"

# include <vector>

class Poll
{
public:
	typedef std::vector<IFileDescriptor *>::size_type size_type;
	
	static void add(IFileDescriptor *src);
	static void add(struct pollfd &src);
	static void remove(size_type pos);
	static void poll();
	static void signalHandler(int);
private:
	bool _stop;
	std::vector<IFileDescriptor *> socket;
	std::vector<struct pollfd> pollfd;

	Poll();
	Poll(Poll const &other);
	~Poll();
	Poll &operator=(Poll const &other);
	void _imtl();
	static Poll &get();
};

#endif //POLL_HPP