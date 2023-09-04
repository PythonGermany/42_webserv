# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: jharrach <@student.42heilbronn.de>         +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/03/16 20:23:47 by jharrach          #+#    #+#              #
#    Updated: 2023/09/04 16:36:51 by jharrach         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		:= webserv

OBJ_DIR		:= obj
SRC_DIR		:= src
INC_DIR		:= include

SRC			+= main.cpp
SRC			+= Address.cpp
SRC			+= Connection.cpp
SRC			+= ListenSocket.cpp
SRC			+= Pollstructs.cpp

HEADERS		+= Address.hpp
HEADERS		+= Connection.hpp
HEADERS		+= ListenSocket.hpp
HEADERS		+= Pollstructs.hpp
HEADERS		+= testconn.hpp

OBJ			:= $(addprefix $(OBJ_DIR)/, $(SRC:%.cpp=%.o))
DEPS		:= $(addprefix $(INC_DIR)/, $(HEADERS))

CXX			:= c++
CXXFLAGS	:= -Wall -Wextra -Werror -std=c++98

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEPS) | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	$(RM) -r $(OBJ_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
