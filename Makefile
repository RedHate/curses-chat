#############################
# RedHate (ultros)
#############################

TARGET      =  chat
OBJ         =  curses-chat.o
LDLIBS		=  -lncurses -lpthread

INCLUDES	?= -I/usr/include
LIBS		?= -L/usr/lib
CFLAGS   	?= -Wall -g -O2 $(INCLUDES) $(LIBS)
CXXFLAGS 	?= -Wall -g -O2 $(INCLUDES) $(LIBS)
CC          =  gcc
CXX         =  g++
LD          =  gcc
MV          =  mv
CP          =  cp
ECHO        =  echo
RM          =  rm
AR          =  ar
RANLIB      =  ranlib
STRIP       =  strip

## console colors
BLACK1      = \033[1;30m
BLACK2      = \033[0;30m
RED1        = \033[1;31m
RED2        = \033[0;31m
GREEN1      = \033[1;32m
GREEN2      = \033[0;32m
YELLOW1     = \033[1;33m
YELLOW2     = \033[0;33m
BLUE1       = \033[1;34m
BLUE2       = \033[0;34m
MAGENTA1    = \033[1;35m
MAGENTA2    = \033[0;35m
CYAN1       = \033[1;36m
CYAN2       = \033[0;36m
WHITE1      = \033[1;37m
WHITE2      = \033[0;37m
NOCOLOR     = \033[0m

.PHONY: all test clean

all: $(OBJ) $(TARGET)

test: $(OBJ) $(TARGET)
	./$(TARGET)

clean: 
	@printf "$(RED1)[CLEANING]$(NOCOLOR)\n" 
	@rm $(OBJ) $(TARGET)

%.o: %.c
	@printf "$(RED1)[CC]$(NOCOLOR) $(notdir $(basename $<)).o\n" 
	@$(CC) -c $< $(CFLAGS) -o $(basename $<).o 

$(TARGET): $(OBJ)
	@printf "$(RED1)[CC]$(NOCOLOR) $(TARGET)\n"
	@$(CC) $(OBJ) $(LDLIBS) -o $(TARGET)
	@$(STRIP) $(TARGET)


