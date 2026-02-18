all:
	gcc curses-chat.c -o curses-chat -lncurses

clean:
	rm -rf curses-chat
