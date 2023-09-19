/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   timeval.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jharrach <jharrach@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/21 16:39:41 by jharrach          #+#    #+#             */
/*   Updated: 2023/09/19 03:34:16 by jharrach         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TIMEVAL_HPP
# define TIMEVAL_HPP

# include <sys/time.h>

timeval operator-(timeval const &lhs, timeval const &rhs);

#endif //TIMEVAL_HPP
