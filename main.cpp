/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plichota <plichota@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/20 15:28:50 by plichota          #+#    #+#             */
/*   Updated: 2026/02/20 15:32:27 by plichota         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <string>


void init(std::string port, std::string password)
{
    if (port.empty() || password.empty())
    {
        std::cerr << "Error: Port and password must not be empty." << std::endl;
        return;
    }
    // parse port

    // parse password
    
    std::cout << "Initializing server on port " << port << std::endl;
    // call server connection
}

// ./ircserv <port> <password>
int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return (0);
    }
    init(argv[1], argv[2]);
}