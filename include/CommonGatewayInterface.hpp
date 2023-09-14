/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   template.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jharrach <@student.42heilbronn.de>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/21 16:39:41 by jharrach          #+#    #+#             */
/*   Updated: 2023/04/21 17:18:40 by jharrach         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMONGATEWAYINTERFACE_HPP
# define COMMONGATEWAYINTERFACE_HPP

# include "AFileDescriptor.hpp"
# include "AConnection.hpp"

# include <string>

class CommonGatewayInterface : public AFileDescriptor
{
public:
    CommonGatewayInterface(AConnection &Connection);
	CommonGatewayInterface(CommonGatewayInterface const &other);
	~CommonGatewayInterface();
	CommonGatewayInterface &operator=(CommonGatewayInterface const &other);

    void send(std::string msg);
private:
    AConnection &_conn;
    std::string _readBuffer;
	// CommonGatewayInterface();
    void onPollIn(struct pollfd &pollfd);
    void onPollOut(struct pollfd &pollfd);
};

#endif //COMMONGATEWAYINTERFACE_HPP
