#include <stdlib.h>
#include "archke_error.h"
#include "archke_socket.h"
#include "archke_event_loop.h"
#include "archke_event_handlers.h"

int main(void) {
	// create socket, open it and make it listen on port
	int serverSocketFd = rchkServerSocketNew(9999);
	if (serverSocketFd < 0) {
		exitFailure("Cannot create/init server socket failed");
	}

	// set server socket IO to non-blocking mode
	if (rchkSocketSetMode(serverSocketFd, ARCHKE_SOCKET_MODE_NON_BLOCKING) < 0) {
		rchkServerSocketClose(serverSocketFd);
		exitFailure("Cannot make server socket non-blocking");
	}

	// create the event loop
	RchkEventLoop* eventLoop = rchkEventLoopNew(512);
  	if (eventLoop == NULL) {
		rchkServerSocketClose(serverSocketFd);
		exitFailure("Cannot create/init main event loop");
  	}

	// register server's socket and "accept" event handler
	RchkClientConfig config = { .data = &serverSocketFd, .free = NULL };
	if (rchkEventLoopRegister(eventLoop, serverSocketFd, ARCHKE_EVENT_LOOP_READ_EVENT, rchkHandleAcceptEvent, &config) < 0) {
		rchkEventLoopFree(eventLoop);
		rchkServerSocketClose(serverSocketFd);
		exitFailure("Cannot register 'connection accept' event handler");
	}

	// run event loop
	if (rchkEventLoopMain(eventLoop) < 0) {
		// TODO: Gracefully disconnect clients and release resources 
		exitFailure("Error during main event loop run");
	}

	rchkEventLoopFree(eventLoop);
	rchkServerSocketClose(serverSocketFd);

	return 0;
}

