#############################
# RedHate (ultros)
#############################

TARGET      =  chat
OBJ         =  curses-chat.o

LDLIBS		=  -lncurses  -lpthread  

PREFIX      =  x86_64-linux-gnu
#PREFIX		=  arm-linux-gnueabihf

CC          =  $(PREFIX)-gcc
CXX         =  $(PREFIX)-g++
LD          =  $(PREFIX)-gcc
MV          =  mv
CP          =  cp
ECHO        =  echo
RM          =  rm
AR          =  $(PREFIX)-ar
RANLIB      =  $(PREFIX)-ranlib
STRIP       =  $(PREFIX)-strip

INCLUDES	?= -I/usr/include
LIBS		?= -L/usr/lib/$(PREFIX)
LDFLAGS		?= -B/usr/lib/$(PREFIX)

CFLAGS   	?= -Wall -g -O2 $(INCLUDES) $(LIBS) -fPIC -no-pie
CXXFLAGS 	?= -Wall -g -O2 $(INCLUDES) $(LIBS) -fPIC -no-pie
WARNINGS	:= -w

## colors are fun
BLACK1		= \033[1;30m
BLACK2		= \033[0;30m
RED1		= \033[1;31m
RED2		= \033[0;31m
GREEN1		= \033[1;32m
GREEN2		= \033[0;32m
YELLOW1		= \033[1;33m
YELLOW		= \033[0;33m
BLUE1		= \033[1;34m
BLUE2		= \033[0;34m
PURPLE1		= \033[1;35m
PURPLE2		= \033[0;35m
CYAN1		= \033[1;36m
CYAN2		= \033[0;36m
WHITE1		= \033[1;37m
WHITE2		= \033[0;37m
NOCOLOR		= \033[0m

.PHONY: all run clean

all: $(ASSETS) $(OBJ) $(RES) $(TARGET)

run-x86_64:  $(ASSETS) $(OBJ) $(RES) $(TARGET)
	@./$(TARGET)-x86_64

run-armhf:  $(ASSETS) $(OBJ) $(RES) $(TARGET)
	@./$(TARGET)-armhf

clean: 
	@printf "$(RED1)[CLEANING]$(NOCOLOR)\n" 
	@rm $(OBJ) $(RES) $(TARGET)-x86_64 $(TARGET)-armhf $(ASSETS)

%.o: %.cpp
	@printf "$(RED1)[CXX]$(NOCOLOR) $(notdir $(basename $<)).o\n" 
	@$(CXX) $(WARNINGS) -c $< $(CXXFLAGS) -o $(basename $<).o 

%.o: %.cxx
	@printf "$(RED1)[CXX]$(NOCOLOR) $(notdir $(basename $<)).o\n" 
	@$(CXX) $(WARNINGS) -c $< $(CFLAGS) -o $(basename $<).o 

%.o: %.c
	@printf "$(RED1)[CC]$(NOCOLOR) $(notdir $(basename $<)).o\n" 
	@$(CC) $(WARNINGS) -c $< $(CFLAGS) -o $(basename $<).o 

%.a:
	@printf "$(RED1)[CC]$(NOCOLOR) $(basename $(TARGET_LIB)).a\n" 
	@$(AR) -cru $(basename $(TARGET_LIB)).a $(OBJ)

## For x86_64-linux-gnu
ifeq ($(PREFIX), x86_64-linux-gnu)
$(TARGET): $(ASSETS) $(OBJ)
	@printf "$(RED1)[CC]$(NOCOLOR) $(TARGET)-x86_64\n"
	@$(CC) $(OBJ) -shared -fPIC  $(LDLIBS) $(LDFLAGS) $(CXXFLAGS) -o $(TARGET)-x86_64
	@$(STRIP) $(TARGET)-x86_64
endif

ifeq ($(PREFIX), arm-linux-gnueabihf)
## For arm-linux-gnueabihf
$(TARGET): $(ASSETS) $(OBJ)
	@printf "$(RED1)[CC]$(NOCOLOR) $(TARGET)-armhf\n"
	@$(CC) $(WARNINGS) -shared -fPIC  $(LDFLAGS) $(OBJ) -o $(TARGET)-armhf
	@$(STRIP) $(TARGET)-armhf
endif
