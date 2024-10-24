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
	rchk_ssr* reader;
};

typedef struct client client;

void error(const char *msg) {
    perror(msg);
    exit(1);
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

void register_in_epoll(int epoll_fd, int client_sock_fd) {
 	struct epoll_event event;

	bzero((char*) &event, sizeof(event));
 	
 	// initialize reader
	rchk_ssr_status status;
	rchk_ssr* reader = rchk_ssr_new(1024, &status);

	// initialize client data
	client* c = malloc(sizeof(client));
	c->fd = client_sock_fd;
	c->reader = reader;

	// initialize event subscription
	event.events = EPOLLIN | EPOLLET; // read event + edge-triggered mode
	event.data.ptr = c;	

	// registration in epoll
	int reg_result = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_sock_fd, &event);  
 	
	if (reg_result == -1) {
 		error("epoll_ctl() error");
 	}
}

// AF_INET - IPv4 protocol
// SOCK_STREAM - Provides sequenced, reliable, two-way, connection-based byte streams.  An out-of-band data transmission mechanism may be supported.	
int main(void) {
	int server_sock_fd;

	// create server socket
	printf("Creating server socket...\n");
	if ((server_sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		error("socket() error");
	}

	// bind server socket to address/port
	printf("Binding server socket...\n");
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
	printf("Making server socket as listener...\n");
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
		// wait for events to come/happen
		int nevents = epoll_wait(epoll_fd, events, MAXEVENTS, -1);
		
		if (nevents == -1) {
			error("epoll_wait() error");
		}		
		
		for (int i=0; i<nevents; i++) {
			client* c = (client*) events[i].data.ptr;

			if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN))) {
				// error case
				fprintf(stderr, "epoll error\n");
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
						printf("Accepted new connection on fd %d ...\n", client_sock_fd);
						
						set_nonblocking(client_sock_fd);

						register_in_epoll(epoll_fd, client_sock_fd);
					} else if (errno == EAGAIN || errno == EWOULDBLOCK){
          				    	// we processed all of pending connections
          				    	break;
					} else {
						error("accept() error");
					}
				}
			}
		        else {
				printf("Reading client input data..\n");
				// client socket; read as much data as we can
				char chunk[256];
        			for (;;) {
					// printf("fd : %d\n", c->fd);
        			 	ssize_t nbytes = read(c->fd, chunk, sizeof(chunk));

					if (nbytes > 0) {	
						rchk_ssr* reader = c->reader;
						rchk_ssr_status status;

						rchk_ssr_process(reader, chunk, nbytes, &status);

						if (rchk_ssr_is_done(reader)) {
							printf("Client %d : %s\n", c->fd, rchk_ssr_str(reader));
							rchk_ssr_clear(reader);
						}
					} else if (nbytes == 0) {
						printf("Finished with %d\n", c->fd);

						shutdown(c->fd, SHUT_WR);
						close(c->fd);

						rchk_ssr_free(c->reader);
						free(c);
        			 	  	
						break;
					} else if (nbytes == -1) {
						if (errno == EAGAIN || errno == EWOULDBLOCK) {
        			 	  	  	printf("Finished reading data from client...\n");
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

	printf("Sockets closed...\n");
     
	return 0;
}

