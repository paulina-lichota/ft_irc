# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: cwannhed <cwannhed@student.42firenze.it    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/03/21 21:08:23 by cwannhed          #+#    #+#              #
#    Updated: 2026/03/21 21:40:52 by cwannhed         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = ircserv

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -Wshadow -std=c++98

SRC = main.cpp \
	Server.cpp

all: $(NAME)

$(NAME): $(SRC)
	@echo "---- Building of $(NAME) ----$(RESET)";
	$(CXX) $(CXXFLAGS) -o $(NAME) $(SRC)

clean:

fclean: clean
	@rm -rf $(NAME)
	@find . -maxdepth 1 -type f -name "*_*" ! -name "*.*" -delete

re: fclean all

.PHONY: all clean fclean re