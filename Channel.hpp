/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plichota <plichota@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/24 17:55:39 by plichota          #+#    #+#             */
/*   Updated: 2026/03/24 17:56:15 by plichota         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include <string>

class Channel
{
    private:
        std::string name;
    public:
        Channel(std::string);
        ~Channel();
        std::string getName();
};

#endif