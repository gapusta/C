#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h> // This header file contains definitions of a number of data types used in system calls. These types are used in the next two include files.
#include <sys/socket.h> 
#include <netinet/in.h>
#include <unistd.h>

void error(const char *msg) {
    perror(msg);
    exit(1);
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

	while(1) {
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
     
		n = write(newsockfd, buffer, n);
     
		if (n < 0) {
			error("ERROR writing to socket");
		}
	}
     
	close(newsockfd);
     	close(sockfd);

	printf("Sockets closed...\n");
     
	return 0;
}

