NAME = webserv

FLAGS = -Wall -Wextra -Werror -std=c++98

SRC = main.cpp Config.cpp Server.cpp Socket.cpp Cgi.cpp Request.cpp \
			Response.cpp File.cpp utils.cpp
OBJ = $(SRC:%.cpp=%.o)

all : $(NAME)

test_config:
	c++ $(FLAGS) -o test_config test_config.cpp Config.cpp Server.cpp Cgi.cpp File.cpp utils.cpp

test_cgi:
	c++ $(FLAGS) -o test_cgi test_cgi.cpp

test_file:
	c++ $(FLAGS) -o test_file test_file.cpp File.cpp

$(NAME) : $(OBJ)
	c++ $(FLAGS) -o $(NAME) $(OBJ)

%.o : %.cpp
	c++ -c $(FLAGS) $^

clean :
	rm -rf $(OBJ)

fclean : clean
	rm -rf $(NAME)

re : fclean all