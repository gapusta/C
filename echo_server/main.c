#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h> // This header file contains definitions of a number of data types used in system calls. These types are used in the next two include files.
#include <sys/socket.h> 
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include "archke_simple_string_reader.h"

#define PORT 9999
#define MAXEVENTS 64 

struct client {
	int fd;
	RchkStringReader* reader;
	int sent;
};

typedef struct client client;

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int min(int x, int y) {
	if (x < y) return x; else return y;
}

void set_nonblocking(int fd) {
  	int flags = fcntl(fd, F_GETFL, 0);
	
	if (flags == -1) {
		error("fcntl() get flags error");    
  	}

  	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
    		error("fcntl() set flags error");
  	}
}

void set_interests(int epoll_fd, client* c, unsigned int interests) {
	struct epoll_event event; bzero((char*) &event, sizeof(event));

	event.events = interests;
	event.data.ptr = c;	

	// registration in epoll
	int reg_result = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, c->fd, &event);  
 	
	if (reg_result == -1) {
 		error("epoll_ctl() error");
 	}
}

void modify_interests(int epoll_fd, client* c, unsigned int interests) {
	struct epoll_event event; bzero((char*) &event, sizeof(event)); 

	event.events = interests;
	event.data.ptr = c;	

	// modify events
	int reg_result = epoll_ctl(epoll_fd, EPOLL_CTL_MOD, c->fd, &event);  
 	
	if (reg_result == -1) {
 		error("epoll_ctl() error");
 	}
}

void register_in_epoll(int epoll_fd, int client_sock_fd) {
 	// initialize reader
	RchkStringReader* reader = rchkStringReaderNew(1024);

	// initialize client data
	client* c = malloc(sizeof(client));
	c->fd = client_sock_fd;
	c->reader = reader;
	c->sent = 0;

	// initialize event subscription : EPOLLIN = read event, EPOLLET = edge-triggered mode
	set_interests(epoll_fd, c, EPOLLIN | EPOLLET);
}

void print_raw_simple_string(char* chunk, int bytes) {
	printf("Chunk : ");
	for (int i=0; i<bytes; i++) {
		if (chunk[i] == '\r') { printf("%s", "\\r"); }
		else if (chunk[i] == '\n') { printf("%s", "\\n"); }
		else { printf("%c", chunk[i]); }
	}
	printf("\n");
}

// AF_INET - IPv4 protocol
// SOCK_STREAM - Provides sequenced, reliable, two-way, connection-based byte streams.  An out-of-band data transmission mechanism may be supported.	
int main(void) {
	int server_sock_fd;

	// create server socket
	printf("Creating server socket\n");
	if ((server_sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		error("socket() error");
	}

	// bind server socket to address/port
	printf("Binding server socket\n");
	struct sockaddr_in server_address; // will contain the address of the server
	bzero((char*) &server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
     	server_address.sin_addr.s_addr = INADDR_ANY;
     	server_address.sin_port = htons(PORT);
	if (bind(server_sock_fd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
		error("bind() error");
	}

	// make server socket nonblocking
	set_nonblocking(server_sock_fd);

	// mark server as "listener"
	printf("Making server socket as listener\n");
	if (listen(server_sock_fd, SOMAXCONN) < 0) { // the server socket will be used to accept incoming connection requests using accept(2)
		error("listen() error");
	}

	// create the epoll
  	int epoll_fd = epoll_create1(0);
  	if (epoll_fd == -1) {
  		error("epoll_create1() error");
  	}

	// mark server socket for reading, and become edge-triggered
	struct epoll_event server_event; 
	bzero((char*) &server_event, sizeof(server_event));
	server_event.data.fd = server_sock_fd;
	server_event.events = EPOLLIN | EPOLLET;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_sock_fd, &server_event) == -1) {
		error("epoll_ctl() error");
	}

	struct epoll_event* events = calloc(MAXEVENTS, sizeof(server_event));
	for(;;) {
		printf("Waiting for IO events\n");
		int nevents = epoll_wait(epoll_fd, events, MAXEVENTS, -1);
		
		if (nevents == -1) {
			error("epoll_wait() error");
		}		
		
		for (int i=0; i<nevents; i++) {
			client* c = (client*) events[i].data.ptr;

			if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)) {
				// error case
				printf("epoll event error or hang up happened\n");
				close(c->fd);
				continue;
			}
			
			if (events[i].data.fd == server_sock_fd) {
				// server socket; call accept as many times as we can
				for (;;) {
					struct sockaddr client_address;
					socklen_t client_address_len = sizeof(client_address);

					int client_sock_fd = accept(server_sock_fd, &client_address, &client_address_len);

					if (client_sock_fd != -1) {
						printf("Accepted new connection/client : %d\n", client_sock_fd);
						
						set_nonblocking(client_sock_fd);
						printf("Set %d as non-blockin\n", client_sock_fd);

						register_in_epoll(epoll_fd, client_sock_fd);
						printf("Registered %d in epoll\n", client_sock_fd);
					} else if (errno == EAGAIN || errno == EWOULDBLOCK){
          				    	// we processed all of pending connections
          				    	break;
					} else {
						error("accept() error");
					}
				}
				continue;
			}

			if (events[i].events & EPOLLIN) {
				char chunk[256];
				printf("Reading message from client : %d\n", c->fd);
				for (;;) {
					// read as much data as we can
					ssize_t nbytes = read(c->fd, chunk, sizeof(chunk));

					if (nbytes > 0) {
						rchkStringReaderProcess(c->reader, chunk, nbytes);

						if (rchkStringReaderIsDone(c->reader)) {
							printf("Finished reading message from : %d, message : %s\n", c->fd, rchkStringReaderData(c->reader));
							modify_interests(epoll_fd, c, EPOLLOUT | EPOLLET);
							break;
						}
					} else if (nbytes == 0) {
						printf("Client %d : exited\n", c->fd);

						shutdown(c->fd, SHUT_WR);
						close(c->fd);

						rchkStringReaderFree(c->reader);
						free(c);

						break;
					} else if (nbytes == -1) {
						if (errno == EAGAIN || errno == EWOULDBLOCK) {
							printf("Client : %d socket input buffer is empty\n", c->fd);
							break;
						} else {
							error("read() error");
						}
					}
				}
				break;
			}

			if (events[i].events & EPOLLOUT) {
				int chunk_size = 256;
     			char chunk[chunk_size];
				char* payload = rchkStringReaderData(c->reader);
				printf("Sending back: %s\n", payload);

				for (;;) {
					// write as much data as we can
					int str_size = rchkStringReaderDataSize(c->reader);
					int occupied = 0;
					int prefix_size = 0;
					int payload_size = 0;
					int suffix_size = 0;

					// 1. compute and set message prefix if needed
					if (c->sent < 1) {
						chunk[occupied++] = '+';
						prefix_size = 1;
					}

					// 2. copy message part to buffer
					int remaining = str_size - c->sent;
					int chunk_available = chunk_size - prefix_size;

					payload_size = min(remaining, chunk_available);

					for (int pidx=0; pidx < payload_size; pidx++, occupied++) {
						chunk[occupied] = payload[c->sent + pidx];
					}

					// 3. deal with '\r'
					if (c->sent < str_size + 2 && occupied < chunk_size) {
						chunk[occupied++] = '\r';
						suffix_size++;
					}

					// 4. deal with '\n'
					if (c->sent < str_size + 3 && occupied < chunk_size) {
						chunk[occupied++] = '\n';
						suffix_size++;
					}

					// send data
					ssize_t nbytes = write(c->fd, chunk, prefix_size + payload_size + suffix_size);

					if (nbytes >= 0) {
						c->sent = c->sent + nbytes;
						
						if (c->sent == str_size + 3) {
							// all the data has been sent
							printf("Finished sending message back. Message : %s\n", rchkStringReaderData(c->reader));
							c->sent = 0;
							rchkStringReaderClear(c->reader);
							modify_interests(epoll_fd, c, EPOLLIN | EPOLLET);
							break;
						}
					} else {
						if (errno == EAGAIN || errno == EWOULDBLOCK) {
							printf("Client : %d socket output buffer is full\n", c->fd);
							break;
						} else {
							error("read() error");
						}
					}
				}
			}
		}
	}
     
	close(server_sock_fd);

	printf("Server socket closed\n");
     
	return 0;
}
