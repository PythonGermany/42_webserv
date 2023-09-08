/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IFileDescriptor.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jharrach <@student.42heilbronn.de>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/21 16:39:41 by jharrach          #+#    #+#             */
/*   Updated: 2023/09/08 05:08:44 by jharrach         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IFILEDESCRIPTOR_HPP
# define IFILEDESCRIPTOR_HPP

# include <poll.h>

class IFileDescriptor
{
public:
	virtual ~IFileDescriptor() {};
	virtual void pollout(struct pollfd &pollfd) = 0;
	virtual void pollin(struct pollfd &pollfd) = 0;
};

#endif //IFILEDESCRIPTOR_HPP
