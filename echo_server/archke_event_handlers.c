#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "archke_socket.h"
#include "archke_event_loop.h"
#include "archke_event_handlers.h"
#include "archke_utils.c"

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void rchkHandleWriteEvent(RchkEventLoop* eventLoop, int fd, struct RchkEvent* event, void* clientData) {
	int chunk_size = 256;
	char chunk[chunk_size];
	Client* c = (Client*) clientData;
	char* payload = rchkStringReaderData(c->reader);
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
	int nbytes = rchkSocketWrite(c->fd, chunk, prefix_size + payload_size + suffix_size);
	if (nbytes < 0) {
		error("read() error");
		return;
	}

	c->sent = c->sent + nbytes;
	
	if (c->sent == str_size + 3) {
		// all the data has been sent
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
	Client* c = (Client*) clientData;
	
	int nbytes = rchkSocketRead(c->fd, chunk, sizeof(chunk));

	if (nbytes == -1) {
		error("read() error");
	}

	if (nbytes == 0) {
		rchkEventLoopUnregister(eventLoop, c->fd);
		rchkSocketClose(c->fd);
		rchkStringReaderFree(c->reader);
		free(c);
		return;
	}

	rchkStringReaderProcess(c->reader, chunk, nbytes);

	if (rchkStringReaderIsDone(c->reader)) {
		// register write handler for client
		int result = rchkEventLoopRegister(eventLoop, c->fd, ARCHKE_EVENT_LOOP_WRITE_EVENT, rchkHandleWriteEvent, c);
		if (result == -1) {
			error("client socket write event registration error");
		}
	}
}

void rchkHandleAcceptEvent(RchkEventLoop* eventLoop, int fd, struct RchkEvent* event, void* clientData) {
	int serverSocketFd = *((int*)clientData);

	int clientSocketFd = rchkServerSocketAccept(serverSocketFd);

	// initialize reader
	RchkStringReader* reader = rchkStringReaderNew(1024);

	// initialize client data
	Client* c = malloc(sizeof(Client));
	c->fd = clientSocketFd;
	c->reader = reader;
	c->sent = 0;

	// register read handler for new client
	int result = rchkEventLoopRegister(eventLoop, clientSocketFd, ARCHKE_EVENT_LOOP_READ_EVENT, rchkHandleReadEvent, c);
	if (result == -1) {
		error("client socket read event registration error");
	}
}

