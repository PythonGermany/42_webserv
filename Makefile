NAME		:= webserv

OBJ_DIR		:= obj
SRC_DIR		:= src
INC_DIR		:= include

SRC			+= main.cpp
SRC			+= global.cpp

SRC			+= poll/Address.cpp
SRC			+= poll/AConnection.cpp
SRC			+= poll/ListenSocket.cpp
SRC			+= poll/Poll.cpp
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
CXXFLAGS	:= -Wall -Wextra -Werror -std=c++98 -Iinclude -Iinclude/poll -Iinclude/config -Iinclude/http -Iinclude/utils -Iinclude/output

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEPS) | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)/poll
	mkdir -p $(OBJ_DIR)/http
	mkdir -p $(OBJ_DIR)/config
	mkdir -p $(OBJ_DIR)/utils
	mkdir -p $(OBJ_DIR)/output

clean:
	$(RM) -r $(OBJ_DIR)

fclean: clean
	rm -f tools/transformer
	$(RM) $(NAME)

re: fclean all

performance:
	make CXXFLAGS="-O3 $(CXXFLAGS)"

debug:
	make CXXFLAGS="-g $(CXXFLAGS)"

fsanitize:
	make CXXFLAGS="-fsanitize=address $(CXXFLAGS)"

custom:
	make CXXFLAGS="$(ARG) $(CXXFLAGS)"

lines:
	@wc -l $(SRC_DIR)/*.cpp $(INC_DIR)/*.hpp $(SRC_DIR)/*/*.cpp $(INC_DIR)/*/*.hpp | sort 

help:
	@echo "Available Makefile rules:"
	@echo "  all          : Build the webserv binary"
	@echo "  clean        : Remove object files"
	@echo "  fclean       : Remove object files and the webserv binary"
	@echo "  re           : Rebuild the webserv binary (fclean + all)"
	@echo "  cgi          : Build the CGI program"
	@echo "  performance  : Build with optimization flags (-O3)"
	@echo "  debug        : Build with debugging symbols (-g)"
	@echo "  custom       : Build with custom CXXFLAGS (e.g., ARG='-DDEBUG')"
	@echo "  flamegraph   : Generate flamegraph profiling SVG"
	@echo "  jmeter       : Download and run Apache JMeter"
	@echo "  lines        : Count lines of code in source files"

.PHONY: all clean fclean re performance debug fsanitize custom flamegraph jmeter lines help
