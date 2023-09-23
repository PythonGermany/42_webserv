NAME		:= test

OBJ_DIR		:= obj
SRC_DIR		:= src
INC_DIR		:= include

SRC			+= unorderedCloseConnectionTest.cpp

OBJ			:= $(addprefix $(OBJ_DIR)/, $(SRC:%.cpp=%.o))
DEPS		:= $(addprefix $(INC_DIR)/, $(HEADERS))


CXX			:= c++
CXXFLAGS	:= -Wall -Wextra -Werror


all: $(NAME)
	@./$<

$(NAME): $(OBJ) $(TEST_OBJ) $(LIBS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEPS) | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $@

clean:
	$(RM) -r $(OBJ_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
