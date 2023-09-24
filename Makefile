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

SRC			+= http/Cache.cpp
SRC			+= http/Http.cpp
SRC			+= http/Request.cpp
SRC			+= http/Response.cpp
SRC			+= http/Uri.cpp

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

HEADERS		+= http/Cache.hpp
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
	rm -f tools/transformer
	$(RM) $(NAME)

re: fclean all

cgi: cgi/cgi

cgi/cgi:
	$(CXX) $(CXXFLAGS) cgi/cgi.cpp -o $@

perfornance:
	make CXXFLAGS="-O3 $(CXXFLAGS)"

debug:
	make CXXFLAGS="-g $(CXXFLAGS)"

lines:
	@wc -l $(SRC_DIR)/*.cpp $(INC_DIR)/*.hpp $(SRC_DIR)/*/*.cpp $(INC_DIR)/*/*.hpp

transformer:
	c++ tools/transformer.cpp -o tools/transformer

tr:
	./tools/transformer r include/* src/* src/*/* include/*/*

tj:
	./tools/transformer j include/* src/* src/*/* include/*/*

.PHONY: all clean fclean re
