NAME	= myShell

CC		= clang++ 
CFLAGS	= -Wall -Wextra -MD

SDIR	= ./srcs/
HDIR	= ./includes/

SRCS	= $(wildcard $(SDIR)*.cpp)
OBJS	= $(SRCS:.cpp=.o)
DEPS	= $(SRCS:.cpp=.d)

all : $(NAME)

$(NAME) : $(OBJS)
	$(CC) -o $(NAME) $(CFLAGS) $(OBJS) -lreadline

%.o : %.cpp
	$(CC) $(CFLAGS) -c -o $@ $< -I$(HDIR)

clean :
	rm -rf $(OBJS) $(DEPS)

fclean : clean
	rm -rf $(NAME)

re : fclean all

-include $(DEPS)

.PHONY : all clean fclean re
