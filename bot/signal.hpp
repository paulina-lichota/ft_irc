/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signal.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plichota <plichota@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/24 16:35:00 by plichota          #+#    #+#             */
/*   Updated: 2026/03/27 15:43:17 by plichota         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SIGNAL_HPP
# define SIGNAL_HPP

#include <csignal>

// extern per non creare copie nuove (e' definita in signal.cpp)
// volatile per renderla non modificabile dal compilatore
//   in questo modo rilegge la variabile "vera" ogni volta che serve invece di tenerla in cache
extern volatile sig_atomic_t received_signal;

void init_signals();
void handle_signal(int signum);

#endif
