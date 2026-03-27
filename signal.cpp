/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signal.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plichota <plichota@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/24 16:35:14 by plichota          #+#    #+#             */
/*   Updated: 2026/03/27 15:57:55 by plichota         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "signal.hpp"
#include <iostream>
#include <cstdlib>

volatile sig_atomic_t received_signal = 0;

void handle_signal(int signum) {
    // std::cout << "\nRicevuto segnale " << signum << std::endl;
    if (signum == SIGINT || signum == SIGTERM)
        received_signal = signum;
}

void init_signals() {
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL); // Gestisce Ctrl+C
    sigaction(SIGTERM, &sa, NULL);  // kill <PID>
}

