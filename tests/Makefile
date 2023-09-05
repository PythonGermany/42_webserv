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

NAME		:= test

OBJ_DIR		:= obj
SRC_DIR		:= ../src
INC_DIR		:= ../include
TEST_DIR	:= src

SRC			+= Address.cpp
TEST_SRC	+= mainTest.cpp

HEADERS		+= Address.hpp

OBJ			:= $(addprefix $(OBJ_DIR)/, $(SRC:%.cpp=%.o))
TEST_OBJ	:= $(addprefix $(OBJ_DIR)/, $(TEST_SRC:%.cpp=%.o))
DEPS		:= $(addprefix $(INC_DIR)/, $(HEADERS))

LIB_DIR		:= googletest/build
LIB_SRC		+= libgtest_main.a
LIB_SRC		+= libgtest.a
LIBS		:= $(addprefix $(LIB_DIR)/lib/, $(LIB_SRC))

CXX			:= c++
CXXFLAGS	:= -Wall -Wextra -Werror -std=c++14 -Igoogletest/googletest/include
LDFLAGS		:= -Lgoogletest/build/lib -pthread $(LIBS)


all: $(NAME)

$(NAME): $(OBJ) $(TEST_OBJ) $(LIBS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEPS) | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(TEST_DIR)/%.cpp $(DEPS) | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $@

$(LIB_DIR):
	mkdir -p $@

$(LIBS): $(LIB_DIR)
	cmake -S googletest -B $<
	make -C $<

clean:
	$(RM) -r $(OBJ_DIR)

fclean: clean
	$(RM) $(NAME)
	$(RM) -r $(LIB_DIR)

re: fclean all

.PHONY: all clean fclean re
