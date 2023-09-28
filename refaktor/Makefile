NAME		:= webserv

OBJ_DIR		:= obj
SRC_DIR		:= src
INC_DIR		:= include

SRC			+= main.cpp

SRC			+= poll/Address.cpp
SRC			+= poll/ListenSocket.cpp
SRC			+= poll/Poll.cpp
SRC			+= poll/CallbackPointer.cpp
SRC			+= poll/AConnection.cpp
SRC			+= poll/timeval.cpp

HEADERS		+= poll/Address.hpp
HEADERS		+= poll/ListenSocket.hpp
HEADERS		+= poll/Poll.hpp
HEADERS		+= poll/IFileDescriptor.hpp
HEADERS		+= poll/CallbackPointer.hpp
HEADERS		+= poll/AConnection.hpp
HEADERS		+= poll/timeval.hpp
HEADERS		+= webserv.hpp

OBJ			:= $(addprefix $(OBJ_DIR)/, $(SRC:%.cpp=%.o))
DEPS		:= $(addprefix $(INC_DIR)/, $(HEADERS))

CXX			:= c++
CXXFLAGS	:= -Wall -Wextra -Werror -std=c++98 -Iinclude -Iinclude/poll -Iinclude/config -Iinclude/http -fsanitize=address

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

performance:
	make CXXFLAGS="-O3 $(CXXFLAGS)"

debug:
	make CXXFLAGS="-g $(CXXFLAGS)"

custom:
	make CXXFLAGS="$(ARG) $(CXXFLAGS)"

flamegraph:
	mkdir -p profiling
	flamegraph -o profiling/flamegraph-$(shell date "+%d-%m-%Y:%H:%M:%S").svg --image-width 1400 --deterministic -- ./$(NAME) $(ARG)

jmeter:
	mkdir -p tests/jmeter
	if [ ! "$(shell ls -A tests/jmeter)" ]; then \
		wget https://dlcdn.apache.org//jmeter/binaries/apache-jmeter-5.6.2.tgz -O tests/jmeter/jmeter.tgz; \
		tar -xzf tests/jmeter/jmeter.tgz -C tests/jmeter --strip-components=1; \
		rm tests/jmeter/jmeter.tgz; \
	fi
	./tests/jmeter/bin/jmeter

lines:
	@wc -l $(SRC_DIR)/*.cpp $(INC_DIR)/*.hpp $(SRC_DIR)/*/*.cpp $(INC_DIR)/*/*.hpp

transformer:
	c++ tools/transformer.cpp -o tools/transformer

tr:
	./tools/transformer r include/* src/* src/*/* include/*/*

tj:
	./tools/transformer j include/* src/* src/*/* include/*/*

.PHONY: all clean fclean re
