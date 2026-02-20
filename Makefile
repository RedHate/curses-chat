
all:
	@gcc -std=c99 -Wall -pedantic  curses-chat.c -o chat -lncurses -lz
	@strip -s chat

clean:
	@rm -rf chat
