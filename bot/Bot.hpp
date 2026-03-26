/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plichota <plichota@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/26 21:47:20 by plichota          #+#    #+#             */
/*   Updated: 2026/03/26 22:08:46 by plichota         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BOT_HPP
#define BOT_HPP

#include <string>

class Bot
{
    private:
        std::string _name;
    public:
        Bot(const std::string &name);
        ~Bot();
        void run();
};

#endif