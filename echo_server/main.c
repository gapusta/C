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

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void set_nonblocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1) {
    perror("fcntl()");
    return;
  }
  if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
    perror("fcntl()");
  }
}

// AF_INET - IPv4 protocol
// SOCK_STREAM - Provides sequenced, reliable, two-way, connection-based byte streams.  An out-of-band data transmission mechanism may be supported.	
int main(void) {
	int sockfd;
	int newsockfd;
        int portno = 9999;
        int clilen;
	int n;

	char buffer[256];

	struct sockaddr_in serv_addr; // will contain the address of the server
       	struct sockaddr_in cli_addr; // will contain the address of the client which connects to the server

	printf("Creating socket...\n");

	// 1. socket creation
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		error("socket() error");
	}

	bzero((char*) &serv_addr, sizeof(serv_addr));

	printf("Binding socket...\n");

	// 2. binding socket to address
	serv_addr.sin_family = AF_INET;
     	serv_addr.sin_addr.s_addr = INADDR_ANY;
     	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		error("bind() error");
	}

	printf("Listening...\n");

	// 3. listen for incoming connections
	listen(sockfd, 5); // marks the socket sockfd as a socket that will be used to accept incoming connection requests using accept(2)
	
	printf("Accepting...\n");
	
	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);
	if (newsockfd < 0) { 
		error("ERROR on accept"); 
	}


	set_nonblocking(newsockfd);

	// create the epoll
  	int epoll_fd = epoll_create1(0);
  	if (epoll_fd == -1) {
  		error("ERROR on epoll_create1");
  	}

	// register client socket with epoll 
	struct epoll_event event;
  	
	bzero((char*) &event, sizeof(event));

	event.data.fd = newsockfd;
  	event.events = EPOLLIN | EPOLLOUT | EPOLLET;
  	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock, &event) == -1) {
  		error("ERROR on epoll_ctl");
	}

	struct epoll_event *events = calloc(64, sizeof(event));
	while(1) {
		int newevents = epoll_wait(epoll_fd, events, 64, -1);
		if (nevents == -1) {
      			error("ERROR on epoll_wait()");
    		}

		for (int i = 0; i < newevents; i++) {
			if (events[i].events & EPOLLIN) {
				// read event fired
				bzero(buffer, 256);

				n = read(newsockfd, buffer, 256);

				if (n < 0) { 
					error("ERROR reading from socket");
				}

				if (n == 0) { // EOF from client (FIN)
					shutdown(newsockfd, SHUT_WR);
					break;
				}

        			buffer[n] = '\0';

				printf("Here is the message: %s\n", buffer);
    
				while (n > 0) {
					int read = write(newsockfd, buffer, n);
					n = n - read;
				}
     
				if (n < 0) {
					error("ERROR writing to socket");
				}
			}
			if (events[i].events & EPOLLOUT) {
				// write event fired
			
			}	
		}

		
	}
     
	close(newsockfd);
     	close(sockfd);

	printf("Sockets closed...\n");
     
	return 0;
}

