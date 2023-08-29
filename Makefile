NAME = webserv

FLAGS = -Wall -Wextra -Werror -std=c++98

SRC_DIR = srcs/
INC_DIR = includes/
OBJ_DIR = .obj/

SRC = main.cpp Config.cpp Server.cpp Context.cpp File.cpp Log.cpp utils.cpp
OBJ = $(addprefix $(OBJ_DIR), $(SRC:%.cpp=%.o))

all : $(OBJ_DIR) $(NAME)

test_file:
	c++ $(FLAGS) -o test_file test_file.cpp File.cpp

$(NAME) : $(OBJ)
	c++ $(FLAGS) -o $(NAME) $^ -I $(INC_DIR)

$(OBJ_DIR)%.o : $(SRC_DIR)%.cpp
	c++ -c $(FLAGS) $^ -o $@ -I $(INC_DIR)

$(OBJ_DIR) :
	mkdir -p $(OBJ_DIR)

lines:
	@wc -l $(SRC_DIR)*.cpp $(INC_DIR)*.hpp

clean :
	rm -rf $(OBJ_DIR)

fclean : clean
	rm -rf $(NAME)

re : fclean all