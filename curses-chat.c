/*
 * Ultros was here in 2026
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <ncurses.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define MAX_MSG 512

// XOR related definitions
#define XOR_KEY_LEN  20
#define XOR_FORWARD  0
#define XOR_BACKWARD 1

// Window ctx handles
WINDOW *chat_win, 
       *input_win;

// Socket var
int sockfd;

// Should suffice with a random key
void xor4x(char *buffer, int size) {
	
	// XOR keys
	unsigned char keys[4][XOR_KEY_LEN] = { 
		{ 0xec, 0x1d, 0x53, 0xdf, 0xc3, 0x23, 0x28, 0xfa, 0xe0, 0xfe, 0x95, 0xc9, 0x51, 0x2a, 0x65, 0x63, 0xe6, 0xb4, 0xf9, 0x3a },
		{ 0x67, 0x10, 0xca, 0x9c, 0x54, 0x66, 0x13, 0xb8, 0x50, 0x67, 0x05, 0x99, 0xa0, 0xaa, 0x53, 0x16, 0xec, 0x5e, 0xcc, 0x3c },
		{ 0x04, 0xf0, 0x24, 0x97, 0x37, 0x23, 0x05, 0x7b, 0x06, 0xe7, 0x0d, 0x48, 0xf2, 0x41, 0x6b, 0xca, 0x55, 0xad, 0x9f, 0xf6 },
		{ 0x87, 0xe9, 0x0a, 0x8c, 0xae, 0x4e, 0x5c, 0xb3, 0xa9, 0x25, 0x7e, 0xa5, 0x34, 0x36, 0x91, 0xf3, 0xdd, 0x1e, 0x1c, 0xc8 },
	};
	
	// Directional XOR fnc (doesn't need to be global, even if that's how you think it should be done. i scope my code.)
	void xor_directional(char *buffer, int size, unsigned char *key, int direction) {
		// Used for xor position
		int keypos = 0;
		// XOR forward / backward
		if(direction == XOR_FORWARD) {
			// Lazy XOR
			for (int i = 0; i < size; i++) {
				// XOR shift the data according to the key and its relative read position
				buffer[i] = (unsigned char)((unsigned char)buffer[i] ^ (unsigned char)key[keypos]);
				// Move the key position
				keypos++;
				// If the key position is greater than the key length reset it
				if(keypos == XOR_KEY_LEN) keypos = 0;
			}
		}
		else if(direction == XOR_BACKWARD) {
			// Lazy XOR
			for (int i = size; i > 0; i--) {
				// XOR shift the data according to the key and its relative read position
				buffer[i] = (unsigned char)((unsigned char)buffer[i] ^ (unsigned char)key[keypos]);
				// Move the key position
				keypos++;
				// If the key position is greater than the key length reset it
				if(keypos == XOR_KEY_LEN) keypos = 0;
			}
		}
	}
	
	// XOR 4x
	int direction = 0;
	// loop through keys
	for(int i = 0; i < 3; i++) {
		// odd
		if(i & 1)
			direction = XOR_BACKWARD;
		// even
		if(!i & 1) // should this be 0? hrm, seems to work.. lol fawk o/ from the non robotic side of me
			direction = XOR_FORWARD;
		
		//xor it in a direction
		xor_directional(buffer, size, &keys[i], direction);
	}
	
}

// Thread to receive messages
void *receive_messages(void *arg) {
    
    // Local vars
    char buffer[MAX_MSG];

	// While running...
    while (1) {
        
        // Get the size of the received bytes from socket
        int bytes = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) {
            break;
        }

		xor4x(&buffer, bytes);

		// Zero the last byte
        buffer[bytes] = '\0';

		// print and refresh the chat window
        wprintw(chat_win, "%s\n", buffer);
        wrefresh(chat_win);
        
    }

	// return
    return NULL;
    
}

// Setup ncurses window
void init_ncurses() {
	
    // Init ncurses
    initscr();
    cbreak();
    //noecho();
    keypad(stdscr, TRUE);
	
	// Local vars
    int height, width;
    getmaxyx(stdscr, height, width);

	// Window context creation
    chat_win = newwin(height - 3, width, 0, 0);
    input_win = newwin(3, width, height - 3, 0);

	// Set scroll property to true
    scrollok(chat_win, TRUE);
    // Create a box around the input window
    box(input_win, 0, 0);

	// Refresh both chat and input windows
    wrefresh(chat_win);
    wrefresh(input_win);
    
}

// Client: %s <server_ip> <port>
int client(int argc, char *argv[]) {
	
	// Print usage
    if (argc != 3) {
        printf("Usage: %s <server_ip> <port>\n", argv[0]);
        return 1;
    }

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

	// Init ncurses
    init_ncurses();

	// Create receive thread
    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, receive_messages, NULL);

	// Create a message buffer for input
    char message[MAX_MSG];
   
    // Create a buffer for user name
    char username[MAX_MSG];

	// erase chat window and print message then refresh
	werase(chat_win);
	wprintw(chat_win, "Insert your user name\n");
	wrefresh(chat_win);
	
	// Erase the input window, replot the input window and refresh
	werase(input_win);
	box(input_win, 0, 0);
	mvwprintw(input_win, 1, 1, "> ");
	wrefresh(input_win);

	// Get the input from keyboard
	wgetnstr(input_win, username, MAX_MSG - 1);
	if(username[0] == 0){
		return 0;
	}

	// Erase and refresh the chat window
	werase(chat_win);
	wrefresh(chat_win);

	// Main loop
    while (1) {
		
		// Erase the input window, replot the input window and refresh
        werase(input_win);
        box(input_win, 0, 0);
        mvwprintw(input_win, 1, 1, "> ");
        wrefresh(input_win);

		// Get the input from keyboard
        wgetnstr(input_win, message, MAX_MSG - 1);

		// Check for quit message
        if (strcmp(message, "/quit") == 0) {
            break;
        }

		else if(strlen(message) > 0) {
			// Combine username and chat message into one
			char combined_buffer[MAX_MSG];
			sprintf(combined_buffer, "%s: %s", username, message);
			
			// Print the message to- and refresh the chat window
			wprintw(chat_win, "%s\n", combined_buffer);
			wrefresh(chat_win);
			
			xor4x(&combined_buffer, strlen(username)+strlen(message)+2);
			
			// Send message to socket
			send(sockfd, combined_buffer, strlen(username)+strlen(message)+2, 0);
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

	// Zero some bytes, set and set fdmax
    FD_ZERO(&master);
    FD_SET(listener, &master);
    fdmax = listener;

	// Print a message
    printf("Server listening on port %d\n", port);

	// Loop the socket management code
    while (1) {
		
        read_fds = master;

        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(1);
        }

		// Loop through all the ports to from 0 - fdmax
        for (int i = 0; i <= fdmax; i++) {
			
			// Is someone attempting to connect?
            if (FD_ISSET(i, &read_fds)) {
				
				// If i is the listener socket
                if (i == listener) {
                    
                    addrlen = sizeof(client_addr);
                    newfd = accept(listener, (struct sockaddr*)&client_addr, &addrlen);

                    if (newfd != -1) {
                        FD_SET(newfd, &master);
                        if (newfd > fdmax)
                            fdmax = newfd;

                        printf("New connection: %s\n", inet_ntoa(client_addr.sin_addr));
                    }

                } 
                else {
					
                    char buffer[MAX_MSG];
                    int nbytes = recv(i, buffer, sizeof(buffer), 0);

                    if (nbytes <= 0) {
                        close(i);
                        FD_CLR(i, &master);
                        printf("Client disconnected\n");
                    } 
                    else {
                        for (int j = 0; j <= fdmax; j++) {
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
	
	// Print usage
    if (argc < 2) {
		printf("Server: %s <port>\n", argv[0]);
        printf("Client: %s <server_ip> <port>\n", argv[0]);
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

