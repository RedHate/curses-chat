/*
 * Ultros was here in 2026
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <ncurses.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define MAX_MSG 512
#define MAX_NAME 16

// Window ctx handles
WINDOW *head_win,
       *chat_win,
       *input_win;

// Socket var
int sockfd;

// chat data type declaration
typedef struct chat_data {
    // Create a buffer for user name
    char username[MAX_NAME];
	// Create a message buffer for input
    char message[MAX_MSG];
}chat_data;

// chat data var declaration
chat_data chatData;

// Should suffice with a random key
int xor4x(unsigned char *buffer, unsigned int size) {
	
	// XOR related definitions
	#define XOR_KEY_LEN  20
	#define XOR_FORWARD  0
	#define XOR_BACKWARD 1
	
	// XOR keys
	unsigned char keys[4][XOR_KEY_LEN] = { 
		{ 0xec, 0x1d, 0x53, 0xdf, 0xc3, 0x23, 0x28, 0xfa, 0xe0, 0xfe, 0x95, 0xc9, 0x51, 0x2a, 0x65, 0x63, 0xe6, 0xb4, 0xf9, 0x3a },
		{ 0x67, 0x10, 0xca, 0x9c, 0x54, 0x66, 0x13, 0xb8, 0x50, 0x67, 0x05, 0x99, 0xa0, 0xaa, 0x53, 0x16, 0xec, 0x5e, 0xcc, 0x3c },
		{ 0x04, 0xf0, 0x24, 0x97, 0x37, 0x23, 0x05, 0x7b, 0x06, 0xe7, 0x0d, 0x48, 0xf2, 0x41, 0x6b, 0xca, 0x55, 0xad, 0x9f, 0xf6 },
		{ 0x87, 0xe9, 0x0a, 0x8c, 0xae, 0x4e, 0x5c, 0xb3, 0xa9, 0x25, 0x7e, 0xa5, 0x34, 0x36, 0x91, 0xf3, 0xdd, 0x1e, 0x1c, 0xc8 },
	};
	
	// Directional XOR fnc (doesn't need to be global, even if that's how you think it should be done. i scope my code.)
	void xor_directional(unsigned char *buffer, unsigned int size, unsigned char *key, int direction) {
		// Used for xor position
		int keypos = 0;
		// XOR forward / backward
		if (direction == XOR_FORWARD) {
			// Lazy XOR
			for (unsigned int i = 0; i < size; i++) {
				// XOR shift the data according to the key and its relative read position
				buffer[i] = (unsigned char)((unsigned char)key[keypos] ^ (unsigned char)buffer[i]);
				// Move the key position
				keypos++;
				// If the key position is greater than the key length reset it
				if (keypos == XOR_KEY_LEN) keypos = 0;
			}
		}
		else if (direction == XOR_BACKWARD) {
			// Lazy XOR
			for (unsigned int i = size; i > 0; i--) {
				// XOR shift the data according to the key and its relative read position
				buffer[i] = (unsigned char)((unsigned char)key[keypos] ^ (unsigned char)buffer[i]);
				// Move the key position
				keypos++;
				// If the key position is greater than the key length reset it
				if (keypos == XOR_KEY_LEN) keypos = 0;
			}
		}
	}
	
	// Set the direction to 0
	int direction = 0;
	
	// loop through keys
	int i;
	for (i = 0; i < 3; i++) {
		// odds
		if (i & 1)
			direction = XOR_FORWARD;
		// evens
		if (!(i & 1))
			direction = XOR_BACKWARD;
		//xor it in a direction
		xor_directional(buffer, size, keys[i], direction);
	}
	
}

// Thread to receive messages
void *receive_messages(void *arg) {
    
    // Local vars
    char buffer[sizeof(chat_data)];

	// While running...
    while (1) {
        
        // Get the size of the received bytes from socket
        int bytes = recv(sockfd, buffer, sizeof(chat_data), 0);
        if (bytes <= 0) {
            break;
        }
		
		// XOR it
		xor4x((unsigned char*)&buffer, sizeof(chat_data));
		
		// Cast it with a pointer
		chat_data *data = (chat_data*)buffer;
        
        // Get the Time
		// Get the Time
		time_t rawtime;
		time(&rawtime);
		char *t_buf = ctime(&rawtime);

		// Print the message to- and refresh the chat window
		wprintw(chat_win, "  recv) [%.8s] <%s>: %s\n", &t_buf[11], data->username, data->message);
		box(chat_win, 0, 0);
		wrefresh(chat_win);
        
    }

	// return
    return NULL;
    
}

// Client: %s <server_ip> <port>
int client(int argc, char *argv[]) {
	
	// Print usage
    if (argc != 3) {
        printf("Usage: %s <server_ip> <port>\n", argv[0]);
        return 1;
    }

	// Init ncurses
    initscr();
    cbreak();
    curs_set(0);
    keypad(stdscr, TRUE);
	
	// Local vars
    int height, width;
    getmaxyx(stdscr, height, width);

	// Window context creation
	head_win  = newwin(3, width, 0, 0);
    chat_win  = newwin(height - 6, width, 3, 0);
    input_win = newwin(3, width, height - 3, 0);

	if (has_colors() != FALSE) {
		start_color();
		init_pair(1, COLOR_MAGENTA, COLOR_BLACK);
		init_pair(2, COLOR_CYAN,    COLOR_BLACK);
		init_pair(3, COLOR_GREEN,   COLOR_BLACK);
		init_pair(4, COLOR_YELLOW,  COLOR_BLACK);
	}
	
	// Set scroll property to true
    scrollok(chat_win, TRUE);
    scrollok(input_win, TRUE);
    
    // Create a box around the head_win
    box(head_win, 0, 0);
    
    // Create a box around the chat_win
    box(chat_win, 0, 0);
    
    // Create a box around the input_win
    box(input_win, 0, 0);
	
	wbkgd(head_win, COLOR_PAIR(1));
	wbkgd(chat_win, COLOR_PAIR(2));
	//wbkgd(input_win, COLOR_PAIR(3));

	// Refresh both chat and input windows
	wrefresh(head_win);
    wrefresh(chat_win);
    wrefresh(input_win);

	// erase header window and print message then refresh
	werase(head_win);
	box(head_win, 0, 0);
	mvwprintw(head_win, 1, 1, " %s ", argv[1]);
	wrefresh(head_win);

	// erase chat window and print message then refresh
	werase(chat_win);
	box(chat_win, 0, 0);
	wrefresh(chat_win);
	
	// Erase the input window, replot the input window and refresh
	werase(input_win);
	box(input_win, 0, 0);
	mvwprintw(input_win, 1, 1, " ");
	wprintw(input_win, "Insert your user name: ");
	wrefresh(input_win);

	// Get the input from keyboard
	char username[MAX_NAME];
	wgetnstr(input_win, username, MAX_NAME - 1);
	
	// Terminate the name just incase it's not terminated
	username[MAX_NAME-1]='\0';

	// Erase and refresh the chat window
	werase(chat_win);
	box(chat_win, 0, 0);
	wprintw(chat_win, "\n");
	wrefresh(chat_win);

	// Server info
    char *server_ip = argv[1];
    int port = atoi(argv[2]);

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket failed");
        return 1;
    }

	// Set address family and port
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);
	
    // Connect to server
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return 1;
    }

	// Create receive thread
    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, receive_messages, NULL);

	// Join message (comes from client not from server ...yet)
	strcpy(chatData.username, username);
	strcpy(chatData.message, "has joined!");

	// XOR it
	xor4x((unsigned char*)&chatData, sizeof(chat_data));
	
	// Send message to socket
	send(sockfd, (const void*)&chatData, sizeof(chat_data), 0);

	// Main loop
    while (1) {
		
		// Place the username into the structured data block
		sprintf(chatData.username, "%s", username);
		
		// erase header window and print message then refresh
		werase(head_win);
		box(head_win, 0, 0);
		mvwprintw(head_win, 1, 1, " %s ", argv[1]);
		wrefresh(head_win);
		
		// Erase the input window, replot the input window and refresh
        werase(input_win);
        box(input_win, 0, 0);
        mvwprintw(input_win, 1, 1, " ");
        wrefresh(input_win);
		
		// Refresh the chat window
		box(chat_win, 0, 0);
		wrefresh(chat_win);
		
		// Get the input from keyboard
        wgetnstr(input_win, chatData.message, MAX_MSG - 1);
        
        //terminate the message data so it never overflows
		chatData.message[MAX_MSG-1] = '\0';
		
        // Check for clear command
        if (strcmp(chatData.message, "/clear") == 0) {
			werase(chat_win);
			box(chat_win, 0, 0);
			wprintw(chat_win, "\n");
			wrefresh(chat_win);
		}
		// Check for quit message
        else if (strcmp(chatData.message, "/newnick") == 0) {
			mvwprintw(input_win, 1, 1, "Insert new nickname: ");
			wrefresh(input_win);
			wgetnstr(input_win, username, MAX_NAME - 1);
			// Terminate the name just incase it's not terminated
			chatData.username[MAX_NAME-1]='\0';
        }
        // No commands? send the input
		else {
			// Check for quit message
			if ((strcmp(chatData.message, "/exit") == 0) || (strcmp(chatData.message, "/quit") == 0)) {
				// Join message (comes from client not from server ...yet)
				strcpy(chatData.username, username);
				strcpy(chatData.message, "has quit!");
				// XOR it
				xor4x((unsigned char*)&chatData,sizeof(chat_data));
				// Send message to socket
				send(sockfd, (const void*)&chatData, sizeof(chat_data), 0);
				// break the loop
				break;
			}
			// If the message is greater than 0
			else if (strlen(chatData.message) > 0) {
				// Get the Time
				time_t rawtime;
				time(&rawtime);
				char *t_buf = ctime(&rawtime);
				// Print the message to- and refresh the chat window
				wprintw(chat_win, "  send) [%.8s] <%s>: %s\n", &t_buf[11], chatData.username, chatData.message);
				wrefresh(chat_win);
				// xor it
				xor4x((unsigned char*)&chatData, sizeof(chat_data));
				// Send message to socket
				send(sockfd, (const void*)&chatData, sizeof(chat_data), 0);
			}
		}
    }

	// Terminate the ncurses windows
    endwin();
    
    // Close socket
    close(sockfd);
    
    // Exit clean
    return 0;
    
}

// Server: %s <port>
int server(int argc, char *argv[]) {
    
    // Check params
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }

	// Local Vars
    int port = atoi(argv[1]);
    int listener, newfd, fdmax;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addrlen;
    fd_set master, read_fds;

	// Create a new socket called listener
    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) {
        perror("socket");
        exit(1);
    }

	// Set the sock option to reuse the socket
    int yes = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

	// Address family, port stuff
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
	
	// Bind the listener socket
    if (bind(listener, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(1);
    }

	// Now listen on the listener socket
    if (listen(listener, 10) < 0) {
        perror("listen");
        exit(1);
    }

	// Zero and set
    FD_ZERO(&master);
    FD_SET(listener, &master);
    fdmax = listener;

	// Print a message
    printf("\033[1;33mServer listening on port %d\033[0m\n", port);

	// Loop the socket management code
    while (1) {
		
        read_fds = master;

		// select the socket
        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(1);
        }
		
		int i;
		// Loop through all the ports to from 0 - fdmax
        for (i = 0; i <= fdmax; i++) {
			
			// Is someone attempting to connect?
            if (FD_ISSET(i, &read_fds)) {
				
				// If i is the listener socket
                if (i == listener) {
                    
                    // Accept a new connection
                    addrlen = sizeof(client_addr);
                    newfd = accept(listener, (struct sockaddr*)&client_addr, &addrlen);

					// Set the socket
                    if (newfd != -1) {
                        FD_SET(newfd, &master);
                        if (newfd > fdmax)
                            fdmax = newfd;
						
						// Print message
                        printf("\033[1;32mClient connected: %s\033[0m\n", inet_ntoa(client_addr.sin_addr));
                    }

                } 
                // Recv and send data
                else {
					
					// Message buffer
                    unsigned char buffer[sizeof(chat_data)];
                    
                    // Recv a message and check if client has disconnected
                    int nbytes = recv(i, buffer, sizeof(buffer), 0);
                    if (nbytes <= 0) {
                        close(i);
                        FD_CLR(i, &master);
                        
                        // Print message
                        printf("\033[1;31mClient disconnected: %s\033[0m\n", inet_ntoa(client_addr.sin_addr));
                    }
                    else {
						// Send received message to all live socket connections 
						int j;
                        for (j = 0; j <= fdmax; j++) {
                            if (FD_ISSET(j, &master) && j != listener && j != i) {
                                send(j, buffer, nbytes, 0);
                            }
                        }
                    }
                }
            }
        }
    }

    return 0;
}

// Main
int main(int argc, char *argv[]) {
	
	/*
	BLACK1		= "\033[1;30m"
	BLACK2		= "\033[0;30m"
	RED1		= "\033[1;31m"
	RED2		= "\033[0;31m"
	GREEN1		= "\033[1;32m"
	GREEN2		= "\033[0;32m"
	YELLOW1		= "\033[1;33m"
	YELLOW		= "\033[0;33m"
	BLUE1		= "\033[1;34m"
	BLUE2		= "\033[0;34m"
	PURPLE1		= "\033[1;35m"
	PURPLE2		= "\033[0;35m"
	CYAN1		= "\033[1;36m"
	CYAN2		= "\033[0;36m"
	WHITE1		= "\033[1;37m"
	WHITE2		= "\033[0;37m"
	NOCOLOR		= "\033[0m"
	*/
	
	// Print usage
    if ((strcmp(argv[1], "-h") == 0) || (argc < 2)) {
		printf("\n    \033[1;36mUltros \033[1;35mMaximus\n    \033[0;32mhttps://gitub.com/redhate\033[0m\n");
		printf("\n    \033[1;33mUsage\033[0m:\n");
		printf("        \033[1;36mServer\033[0m:\n            %s <listener_port>\n", argv[0]);
        printf("        \033[1;35mClient\033[0m:\n            %s <server_ip> <port>\n\n", argv[0]);
        return 1;
    }

	// If argc is 2, it's server mode
	if (argc == 2)
		server(argc, argv);
	// If argc == 3, it's client mode
	if (argc == 3)
		client(argc, argv);
    
    // Exit clean
    return 0;
    
}

