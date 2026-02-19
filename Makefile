all:
	@gcc curses-chat.c -o chat -lncurses -lz
	@strip -s chat

clean:
	@rm -rf chat
