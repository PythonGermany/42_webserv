NAME = webserv

FLAGS = -Wall -Wextra -Werror -std=c++98

SRC = main.cpp Config.cpp Server.cpp Socket.cpp Request.cpp Response.cpp \
			File.cpp utils.cpp
OBJ = $(SRC:%.cpp=%.o)

all : $(NAME)

test_config:
	c++ $(FLAGS) test_config.cpp Config.cpp Server.cpp File.cpp utils.cpp

$(NAME) : $(OBJ)
	c++ $(FLAGS) -o $(NAME) $(OBJ)

%.o : %.cpp
	c++ -c $(FLAGS) $^

clean :
	rm -rf $(OBJ)

fclean : clean
	rm -rf $(NAME)

re : fclean all