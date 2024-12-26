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
#include "archke_logs.h"

#define ARCHKE_WRITE_MAX_OUTPUTS 3

// TODO: Test this function
void rchkHandleWriteEvent(RchkEventLoop* eventLoop, int fd, struct RchkEvent* event, void* clientData) {
	Client* client = (Client*) clientData;

	char* prefix = "+";
	int prefixSize = strlen(prefix);

	char* suffix = "\r\n";
	int suffixSize = strlen(suffix);

	char* payload = rchkStringReaderData(client->reader);
	int payloadSize = rchkStringReaderDataSize(client->reader);
	
	int outputs = 0;
	RchkSocketBuffer buffs[ARCHKE_WRITE_MAX_OUTPUTS];

	if (client->sent < prefixSize) {
		int sentSize = client->sent;

		buffs[outputs].buffer = prefix + sentSize;
		buffs[outputs].size = prefixSize - sentSize;
		outputs++;
	}

	if (client->sent < prefixSize + payloadSize) {
		int sentSize = 0; 
		
		if (client->sent > prefixSize) {
			sentSize = client->sent - prefixSize;
		}

		buffs[outputs].buffer = payload + sentSize;
		buffs[outputs].size = payloadSize - sentSize;
		outputs++;
	}

	if (client->sent < prefixSize + payloadSize + suffixSize) {
		int sentSize = 0; 
		
		if (client->sent > prefixSize + payloadSize) {
			sentSize = client->sent - (prefixSize + payloadSize);
		}

		buffs[outputs].buffer = suffix + sentSize;
		buffs[outputs].size = suffixSize - sentSize;
		outputs++;
	}

	int nbytes = rchkSocketWritev(client->fd, buffs, outputs);
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

	// check if all the data has been sent
	if (client->sent == prefixSize + payloadSize + suffixSize) {
		client->sent = 0;
		rchkStringReaderClear(client->reader);
		// register read handler for client
		RchkClientConfig config = { .data = client, .free = NULL };
		if (rchkEventLoopRegister(eventLoop, client->fd, ARCHKE_EVENT_LOOP_READ_EVENT, rchkHandleReadEvent, &config) < 0) {
			logError("Client socket read event registration error");
			rchkSocketShutdown(client->fd);
			rchkEventLoopUnregister(eventLoop, client->fd);
			rchkSocketClose(client->fd);
			rchkStringReaderFree(client->reader);
			free(client);
			return;
		}
	}
}

#undef ARCHKE_WRITE_MAX_OUTPUTS

void rchkHandleReadEvent(RchkEventLoop* eventLoop, int fd, struct RchkEvent* event, void* clientData) {
	char buffer[256];
	Client* client = (Client*) clientData;
	
	int nbytes = rchkSocketRead(client->fd, buffer, sizeof(buffer));

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

	// client sent us FIN and we received it (client is waiting for us to send FIN back)
	// client will not send us any more data
	if (nbytes == 0) {
		// we send FIN(or possibly FIN,ACK) back (or rather we ask the kernel to send FIN back to client)
		rchkSocketShutdownWrite(client->fd);
		// then we close (release resources)
		rchkSocketClose(client->fd);
		rchkEventLoopUnregister(eventLoop, client->fd);
		rchkStringReaderFree(client->reader);
		free(client);
		return;
	}

	rchkStringReaderProcess(client->reader, buffer, nbytes);

	if (rchkStringReaderIsDone(client->reader)) {
		// register write handler for client
		RchkClientConfig config = { .data = client, .free = NULL };
		if (rchkEventLoopRegister(eventLoop, client->fd, ARCHKE_EVENT_LOOP_WRITE_EVENT, rchkHandleWriteEvent, &config) < 0) {
			logError("Client socket write event registration error");
			rchkSocketShutdown(client->fd);
			rchkEventLoopUnregister(eventLoop, client->fd);
			rchkSocketClose(client->fd);
			rchkStringReaderFree(client->reader);
			free(client);
			return;
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
	RchkClientConfig config = { .data = client, .free = NULL }; // TODO: implement 'free()'
	if (rchkEventLoopRegister(eventLoop, clientSocketFd, ARCHKE_EVENT_LOOP_READ_EVENT, rchkHandleReadEvent, &config) < 0) {
		free(reader);
		free(client);
		rchkSocketClose(clientSocketFd);
		logError("Client event registration failed");
	}

}

