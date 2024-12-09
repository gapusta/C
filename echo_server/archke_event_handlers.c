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
#include "archke_logs.h"

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void rchkHandleWriteEvent(RchkEventLoop* eventLoop, int fd, struct RchkEvent* event, void* clientData) {
	int chunk_size = 256;
	char chunk[chunk_size];
	Client* client = (Client*) clientData;
	char* payload = rchkStringReaderData(client->reader);
	int str_size = rchkStringReaderDataSize(client->reader);
	int occupied = 0;
	int prefix_size = 0;
	int payload_size = 0;
	int suffix_size = 0;

	// 1. compute and set message prefix if needed
	if (client->sent < 1) {
		chunk[occupied++] = '+';
		prefix_size = 1;
	}

	// 2. copy message part to buffer
	int remaining = str_size - client->sent;
	int chunk_available = chunk_size - prefix_size;

	payload_size = min(remaining, chunk_available);

	for (int pidx=0; pidx < payload_size; pidx++, occupied++) {
		chunk[occupied] = payload[client->sent + pidx];
	}

	// 3. deal with '\r'
	if (client->sent < str_size + 2 && occupied < chunk_size) {
		chunk[occupied++] = '\r';
		suffix_size++;
	}

	// 4. deal with '\n'
	if (client->sent < str_size + 3 && occupied < chunk_size) {
		chunk[occupied++] = '\n';
		suffix_size++;
	}

	// send data
	int nbytes = rchkSocketWrite(client->fd, chunk, prefix_size + payload_size + suffix_size);
	if (nbytes < 0) {
		logError("Write to client failed");
		// close connections (exactly how its handled in Redis. See networking.c -> (freeClient(c) -> unlinkClient(c)))
		rchkSocketShutdown(client->fd);
		rchkEventLoopUnregister(eventLoop, client->fd);
		rchkSocketClose(client->fd);
		// free resources
		rchkStringReaderFree(client->reader);
		free(client);
		return;
	}

	client->sent = client->sent + nbytes;
	
	if (client->sent == str_size + 3) {
		// all the data has been sent
		client->sent = 0;
		rchkStringReaderClear(client->reader);
		// register read handler for new client
		int result = rchkEventLoopRegister(eventLoop, client->fd, ARCHKE_EVENT_LOOP_READ_EVENT, rchkHandleReadEvent, client);
		if (result == -1) {
			error("client socket read event registration error");
		}
	}
}

void rchkHandleReadEvent(RchkEventLoop* eventLoop, int fd, struct RchkEvent* event, void* clientData) {
	char chunk[256];
	Client* client = (Client*) clientData;
	
	int nbytes = rchkSocketRead(client->fd, chunk, sizeof(chunk));

	if (nbytes < 0) {
		logError("Read from client failed");
		// close connections (exactly how its handled in Redis. See networking.c -> (freeClient(c) -> unlinkClient(c)))
		rchkSocketShutdown(client->fd);
		rchkEventLoopUnregister(eventLoop, client->fd);
		rchkSocketClose(client->fd);
		// free resources
		rchkStringReaderFree(client->reader);
		free(client);
		return;
	}

	if (nbytes == 0) {
		// client closed the connection
		rchkSocketShutdownWrite(client->fd);
		rchkSocketClose(client->fd);
		rchkEventLoopUnregister(eventLoop, client->fd);
		rchkStringReaderFree(client->reader);
		free(client);
		return;
	}

	rchkStringReaderProcess(client->reader, chunk, nbytes);

	if (rchkStringReaderIsDone(client->reader)) {
		// register write handler for client
		int result = rchkEventLoopRegister(eventLoop, client->fd, ARCHKE_EVENT_LOOP_WRITE_EVENT, rchkHandleWriteEvent, client);
		if (result == -1) {
			error("client socket write event registration error");
		}
	}
}

void rchkHandleAcceptEvent(RchkEventLoop* eventLoop, int fd, struct RchkEvent* event, void* clientData) {
	int serverSocketFd = *((int*)clientData);

	int clientSocketFd = rchkServerSocketAccept(serverSocketFd);
	if (clientSocketFd < 0) {
		logError("Accept client connection failed"); 
		return;
	}

	if (rchkSocketSetMode(clientSocketFd, ARCHKE_SOCKET_MODE_NON_BLOCKING) < 0) {
		rchkSocketClose(clientSocketFd);
		logError("Make client socket non-blocking failed");
		return;
	}

	// initialize reader
	RchkStringReader* reader = rchkStringReaderNew(1024);
	if (reader == NULL) {
		rchkSocketClose(clientSocketFd);
		logError("Client data init failed");
		return;
	}

	// initialize client data
	Client* client = malloc(sizeof(Client));
	if (reader == NULL) {
		free(reader);
		rchkSocketClose(clientSocketFd);
		logError("Client data init failed");
		return;
	}

	client->fd = clientSocketFd;
	client->reader = reader;
	client->sent = 0;

	// register read handler for new client
	if (rchkEventLoopRegister(eventLoop, clientSocketFd, ARCHKE_EVENT_LOOP_READ_EVENT, rchkHandleReadEvent, client) < 0) {
		free(reader);
		free(client);
		rchkSocketClose(clientSocketFd);
		logError("Client event registration failed");
	}

}

