# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: jharrach <jharrach@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/03/16 20:23:47 by jharrach          #+#    #+#              #
#    Updated: 2023/09/19 03:44:05 by jharrach         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		:= webserv

OBJ_DIR		:= obj
SRC_DIR		:= src
INC_DIR		:= include

SRC			+= main.cpp

SRC			+= poll/Address.cpp
SRC			+= poll/AConnection.cpp
SRC			+= poll/ListenSocket.cpp
SRC			+= poll/Poll.cpp
SRC			+= poll/ResponsePipe.cpp
SRC			+= poll/RequestPipe.cpp
SRC			+= poll/timeval.cpp

SRC			+= config/Config.cpp
SRC			+= config/Context.cpp
SRC			+= config/File.cpp
SRC			+= config/Init.cpp
SRC			+= config/Log.cpp
SRC			+= config/structure.cpp
SRC			+= config/utils.cpp
SRC			+= config/VirtualHost.cpp

SRC			+= http/Http.cpp
SRC			+= http/Request.cpp
SRC			+= http/Response.cpp

HEADERS		+= webserv.hpp

HEADERS		+= config/colors.hpp
HEADERS		+= config/Config.hpp
HEADERS		+= config/Context.hpp
HEADERS		+= config/File.hpp
HEADERS		+= config/Init.hpp
HEADERS		+= config/Log.hpp
HEADERS		+= config/structure.hpp
HEADERS		+= config/utils.hpp
HEADERS		+= config/VirtualHost.hpp

HEADERS		+= poll/Address.hpp
HEADERS		+= poll/AConnection.hpp
HEADERS		+= poll/ListenSocket.hpp
HEADERS		+= poll/Poll.hpp
HEADERS		+= poll/IFileDescriptor.hpp
HEADERS		+= poll/ResponsePipe.hpp
HEADERS		+= poll/RequestPipe.hpp
HEADERS		+= poll/timeval.hpp

HEADERS		+= http/testconn.hpp
HEADERS		+= http/Http.hpp
HEADERS		+= http/Request.hpp
HEADERS		+= http/Response.hpp

OBJ			:= $(addprefix $(OBJ_DIR)/, $(SRC:%.cpp=%.o))
DEPS		:= $(addprefix $(INC_DIR)/, $(HEADERS))

CXX			:= c++
CXXFLAGS	:= -Wall -Wextra -Werror -std=c++98 -Iinclude -Iinclude/poll -Iinclude/config -Iinclude/http

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEPS) | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)/poll
	mkdir -p $(OBJ_DIR)/http
	mkdir -p $(OBJ_DIR)/config

clean:
	$(RM) -r $(OBJ_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all

cgi: cgi/cgi

cgi/cgi:
	$(CXX) $(CXXFLAGS) cgi/cgi.cpp -o $@

lines:
	@wc -l $(SRC_DIR)/*.cpp $(INC_DIR)/*.hpp $(SRC_DIR)/*/*.cpp $(INC_DIR)/*/*.hpp

.PHONY: all clean fclean re
