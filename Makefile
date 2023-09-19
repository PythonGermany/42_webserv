NAME = webserv

FLAGS = -Wall -Wextra -Werror -std=c++98

SRC_DIR = src/
INC_DIR = include/
OBJ_DIR = .obj/

SRC = main.cpp Config.cpp Context.cpp VirtualHost.cpp Init.cpp File.cpp Log.cpp Request.cpp Response.cpp structure.cpp utils.cpp
SRC += Poll.cpp Address.cpp ListenSocket.cpp AConnection.cpp Http.cpp
OBJ = $(addprefix $(OBJ_DIR), $(SRC:%.cpp=%.o))

all : $(OBJ_DIR) $(NAME)

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