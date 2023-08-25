NAME = webserv

FLAGS = -Wall -Wextra -Werror -std=c++98

SRC = main.cpp Server.cpp Socket.cpp Request.cpp Response.cpp utils.cpp
OBJ = $(SRC:%.cpp=%.o)

all : $(NAME)

$(NAME) : $(OBJ)
	c++ $(FLAGS) -o $(NAME) $(OBJ)

%.o : %.cpp
	c++ -c $(FLAGS) $^

clean :
	rm -rf $(OBJ)

fclean : clean
	rm -rf $(NAME)

re : fclean all