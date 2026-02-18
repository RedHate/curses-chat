all:
	@gcc curses-chat.c -o curses-chat -lncurses -lz
	@strip -s curses-chat

clean:
	@rm -rf curses-chat
