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
#include "archke_event_loop.h"
#include "archke_simple_string_reader.h"
#include "utils.c"

#define PORT 9999

struct client {
	int fd;
	RchkStringReader* reader;
	int sent;
};

typedef struct client client;
typedef struct server server;

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

void rchkHandleReadEvent(RchkEventLoop* eventLoop, int fd, struct RchkEvent* event, void* clientData);

void rchkHandleWriteEvent(RchkEventLoop* eventLoop, int fd, struct RchkEvent* event, void* clientData) {
	int chunk_size = 256;
	char chunk[chunk_size];
	client* c = (client*) clientData;
	char* payload = rchkStringReaderData(c->reader);
	int str_size = rchkStringReaderDataSize(c->reader);
	int occupied = 0;
	int prefix_size = 0;
	int payload_size = 0;
	int suffix_size = 0;

	printf("Sending back: %s\n", payload);

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
	if (nbytes < 0) {
		error("read() error");
		return;
	}

	c->sent = c->sent + nbytes;
	
	if (c->sent == str_size + 3) {
		// all the data has been sent
		printf("Finished sending message back. Message : %s\n", rchkStringReaderData(c->reader));
		c->sent = 0;
		rchkStringReaderClear(c->reader);
		// register read handler for new client
		int result = rchkEventLoopRegister(eventLoop, c->fd, ARCHKE_EVENT_LOOP_READ_EVENT, rchkHandleReadEvent, c);
		if (result == -1) {
			error("client socket read event registration error");
		}
	}
}

void rchkHandleReadEvent(RchkEventLoop* eventLoop, int fd, struct RchkEvent* event, void* clientData) {
	char chunk[256];
	client* c = clientData;
	
	ssize_t nbytes = read(c->fd, chunk, sizeof(chunk));

	if (nbytes == -1) {
		error("read() error");
	}

	if (nbytes == 0) {
		printf("Client %d : exited\n", c->fd);

		shutdown(c->fd, SHUT_WR);
		close(c->fd);

		rchkStringReaderFree(c->reader);
		free(c);
		return;
	}

	rchkStringReaderProcess(c->reader, chunk, nbytes);

	if (rchkStringReaderIsDone(c->reader)) {
		printf("Finished reading message from : %d, message : %s\n", c->fd, rchkStringReaderData(c->reader));

		// register write handler for client
		int result = rchkEventLoopRegister(eventLoop, c->fd, ARCHKE_EVENT_LOOP_WRITE_EVENT, rchkHandleWriteEvent, c);
		if (result == -1) {
			error("client socket write event registration error");
		}
	}
}

void rchkHandleAcceptEvent(RchkEventLoop* eventLoop, int fd, struct RchkEvent* event, void* clientData) {
	int serverSocketFd = *((int*)clientData);

	struct sockaddr clientAddress;
	socklen_t clientAddrLen = sizeof(clientAddress);
	int clientSocketFd = accept(serverSocketFd, &clientAddress, &clientAddrLen);
	if (clientSocketFd == -1) {
		error("accept() error");
	}

	printf("Accepted new connection/client : %d\n", clientSocketFd);
	
	set_nonblocking(clientSocketFd);
	printf("Set %d as non-blockin\n", clientSocketFd);

	// initialize reader
	RchkStringReader* reader = rchkStringReaderNew(1024);

	// initialize client data
	client* c = malloc(sizeof(client));
	c->fd = clientSocketFd;
	c->reader = reader;
	c->sent = 0;

	// register read handler for new client
	int result = rchkEventLoopRegister(eventLoop, clientSocketFd, ARCHKE_EVENT_LOOP_READ_EVENT, rchkHandleReadEvent, c);
	if (result == -1) {
		error("client socket read event registration error");
	}

	printf("Registered %d in epoll\n", clientSocketFd);
}

int main(void) {
	int serverSocketFd;

	// create server socket
	printf("Creating server socket\n");
	if ((serverSocketFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		error("socket() error");
	}

	// bind server socket to address/port
	printf("bind socket to port 9999\n");
	struct sockaddr_in server_address; // will contain the address of the server
	bzero((char*) &server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
     	server_address.sin_addr.s_addr = INADDR_ANY;
     	server_address.sin_port = htons(PORT);
	if (bind(serverSocketFd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
		error("bind() error");
	}

	// make server socket nonblocking
	printf("set server socket non-blocking\n");
	set_nonblocking(serverSocketFd);

	// mark server as "listener"
	printf("make server socket listen to port 9999\n");
	if (listen(serverSocketFd, SOMAXCONN) < 0) { // the server socket will be used to accept incoming connection requests using accept(2)
		error("listen() error");
	}

	// create the epoll
	printf("Created an event loop\n");
	RchkEventLoop* eventLoop = rchkEventLoopNew(512);
  	if (eventLoop == NULL) {
  		error("Event loop creation error");
  	}

	// register server's socket and "accept" event handler
	printf("Registering server socket accept event\n");
	int result = rchkEventLoopRegister(eventLoop, serverSocketFd, ARCHKE_EVENT_LOOP_READ_EVENT, rchkHandleAcceptEvent, &serverSocketFd);
	if (result == -1) {
		error("server socket accept registration error");
	}

	// run event loop
	printf("Run event loop\n");
	rchkEventLoopMain(eventLoop);

	close(serverSocketFd);

	printf("Server socket closed\n");
     
	return 0;
}

