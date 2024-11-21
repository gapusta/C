#ifndef ARCHKE_ECHO_SERVER_EVENT_HANDLERS
#define ARCHKE_ECHO_SERVER_EVENT_HANDLERS

#include "archke_event_loop.h"
#include "archke_simple_string_reader.h"

typedef struct Client {
	int fd;
	RchkStringReader* reader;
	int sent;
} Client;

// all functions here follow rchkHandleEvent() typedefs signature
void rchkHandleAcceptEvent(RchkEventLoop* eventLoop, int fd, struct RchkEvent* event, void* clientData);
void rchkHandleReadEvent(RchkEventLoop* eventLoop, int fd, struct RchkEvent* event, void* clientData);
void rchkHandleWriteEvent(RchkEventLoop* eventLoop, int fd, struct RchkEvent* event, void* clientData);

#endif

