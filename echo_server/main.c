#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "archke_socket.h"
#include "archke_event_loop.h"
#include "archke_event_handlers.h"

#define PORT 9999

int main(void) {
	// create socket, open it and make listen on port
	int serverSocketFd = rchkServerSocketNew(9999);

	rchkSocketSetMode(serverSocketFd, ARCHKE_SOCKET_MODE_NON_BLOCKING);

	// create the epoll
	RchkEventLoop* eventLoop = rchkEventLoopNew(512);
  	if (eventLoop == NULL) {
		perror("Event loop creation error");
    	exit(1);
  	}

	// register server's socket and "accept" event handler
	int result = rchkEventLoopRegister(eventLoop, serverSocketFd, ARCHKE_EVENT_LOOP_READ_EVENT, rchkHandleAcceptEvent, &serverSocketFd);
	if (result == -1) {
		perror("server socket accept registration error");
    	exit(1);
	}

	// run event loop
	rchkEventLoopMain(eventLoop);

	rchkEventLoopFree(eventLoop);
	rchkServerSocketClose(serverSocketFd);

	return 0;
}

