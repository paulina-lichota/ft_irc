/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plichota <plichota@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/26 22:00:15 by plichota          #+#    #+#             */
/*   Updated: 2026/03/26 22:21:13 by plichota         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Bot.hpp"
#include <iostream>
#include <cstdlib>

// ./bot <port> <password> <channel>
int main(int argc, char **argv)
{
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <port> <password> <channel>" << std::endl;
        return (1);
    }
    try
    {
        // port non e' un numero
        if (!Bot::isValidPort(argv[1]))
            throw std::invalid_argument("Error: invalid port.");
        Bot bot(std::atoi(argv[1]), argv[2], argv[3]);
        bot.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return (1);
    }
    return (0);
}