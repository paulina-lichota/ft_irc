/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   includes.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cwannhed <cwannhed@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/21 21:17:58 by cwannhed          #+#    #+#             */
/*   Updated: 2026/03/24 14:12:25 by cwannhed         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef INCLUDES_HPP
# define INCLUDES_HPP

# include <string>
# include <iostream>
# include <cstdlib> // std::atoi
# include <stdexcept> // std::runtime_error, std::invalid_argument
# include <map>
# include <vector>
# include <sys/socket.h> // socket(), bind(), listen(), accept()
# include <unistd.h> // close()
# include <cstring> // std::memset
# include <netinet/in.h> // struct sockaddr_in
# include <fcntl.h> // fcntl()
# include <poll.h> // poll(), struct pollfd
# include <cerrno> // errno
# include <arpa/inet.h> // inet_ntoa()

# define IRC_MSG_MAX_LEN	512

#endif