NAME		:= webserv
CONF  := project.conf

OBJ_DIR		:= obj
SRC_DIR		:= src
INC_DIR		:= include
BIN_DIR		:= bin
CONF_DIR	:= conf

SRC			+= main.cpp
SRC			+= global.cpp

SRC			+= poll/Address.cpp
SRC			+= poll/AConnection.cpp
SRC			+= poll/ListenSocket.cpp
SRC			+= poll/Poll.cpp
SRC			+= poll/IFileDescriptor.cpp
SRC			+= poll/CallbackPointer.cpp
SRC			+= poll/timeval.cpp

SRC			+= config/argument.cpp
SRC			+= config/Config.cpp
SRC			+= config/Context.cpp
SRC			+= config/Init.cpp

SRC			+= http/Http.cpp
SRC			+= http/Request.cpp
SRC			+= http/Response.cpp
SRC			+= http/Uri.cpp
SRC			+= http/VirtualHost.cpp

SRC			+= output/output.cpp
SRC			+= output/Log.cpp

SRC			+= utils/utils.cpp
SRC			+= utils/File.cpp

HEADERS		+= webserv.hpp
HEADERS		+= global.hpp

HEADERS		+= config/argument.hpp
HEADERS		+= config/Config.hpp
HEADERS		+= config/Context.hpp
HEADERS		+= config/Init.hpp

HEADERS		+= poll/Address.hpp
HEADERS		+= poll/AConnection.hpp
HEADERS		+= poll/ListenSocket.hpp
HEADERS		+= poll/Poll.hpp
HEADERS		+= poll/IFileDescriptor.hpp
HEADERS		+= poll/CallbackPointer.hpp
HEADERS		+= poll/timeval.hpp

HEADERS		+= http/Http.hpp
HEADERS		+= http/Request.hpp
HEADERS		+= http/Response.hpp
HEADERS		+= http/VirtualHost.hpp

HEADERS		+= output/colors.hpp
HEADERS		+= output/Log.hpp
HEADERS		+= output/output.hpp

HEADERS		+= utils/File.hpp
HEADERS		+= utils/utils.hpp

OBJ			:= $(addprefix $(OBJ_DIR)/, $(SRC:%.cpp=%.o))
DEPS		:= $(addprefix $(INC_DIR)/, $(HEADERS))

CXX			:= c++
CXXFLAGS	:= -Wall -Wextra -Werror -Wpedantic -std=c++98 -Iinclude -Iinclude/poll -Iinclude/config -Iinclude/http -Iinclude/utils -Iinclude/output

all: $(BIN_DIR)/$(NAME)

$(BIN_DIR)/$(NAME): $(OBJ) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEPS) | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)/poll
	mkdir -p $(OBJ_DIR)/http
	mkdir -p $(OBJ_DIR)/config
	mkdir -p $(OBJ_DIR)/utils
	mkdir -p $(OBJ_DIR)/output

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	$(RM) -r $(OBJ_DIR)

fclean: clean
	$(RM) -r $(BIN_DIR)

re: fclean all

performance:
	make CXXFLAGS="-O3 $(CXXFLAGS)"

debug:
	make CXXFLAGS="-g $(CXXFLAGS)"

fsanitize:
	make CXXFLAGS="-fsanitize=address $(CXXFLAGS)"

custom:
	make CXXFLAGS="$(ARG) $(CXXFLAGS)"

run: all
	$(BIN_DIR)/$(NAME) $(CONF_DIR)/$(CONF)

docker.build:
	docker build -t webserv .

docker.run:
	docker run --rm -p 8080:80 --name webserv webserv:latest

bear: fclean
	bear -- make

lines:
	@wc -l $(SRC_DIR)/*.cpp $(INC_DIR)/*.hpp $(SRC_DIR)/*/*.cpp $(INC_DIR)/*/*.hpp | sort 

help:
	@echo "Available Makefile rules:"
	@echo "  all          : Build the webserv binary"
	@echo "  clean        : Remove object files"
	@echo "  fclean       : Remove object files and the webserv binary"
	@echo "  re           : Rebuild the webserv binary (fclean + all)"
	@echo "  performance  : Build with optimization flags (-O3)"
	@echo "  debug        : Build with debugging symbols (-g)"
	@echo "  custom       : Build with custom CXXFLAGS (e.g., ARG='-DDEBUG')"
	@echo "  run          : Run webserv binary with project configuration file"
	@echo "  docker.build : Build webserv docker image"
	@echo "  docker.run   : Run webserv docker image"
	@echo "  bear         : Use bear to generate compile_commands.json file which can be used by clangd"
	@echo "  lines        : Count lines of code in source files"

.PHONY: all clean fclean re performance debug fsanitize custom flamegraph jmeter lines help
