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

all: testHeader

testHeader:
	c++ testHeader.cpp -o testHeader.out -I../include/http -I../include/utils -I../include/output

fclean:
	rm -f *.out

re: fclean all

.PHONY: all fclean re testHeader