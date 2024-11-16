#ifndef ARCHKE_EVENT_LOOP
#define ARCHKE_EVENT_LOOP

#define ARCHKE_EVENT_LOOP_NONE_EVENT  0
#define ARCHKE_EVENT_LOOP_READ_EVENT  1
#define ARCHKE_EVENT_LOOP_WRITE_EVENT 2

struct RchkEventLoop;
struct RchkEvent;

typedef void rchkHandleEvent(struct RchkEventLoop* eventLoop, int fd, struct RchkEvent* event, void* clientData);

typedef struct RchkEvent {
    int mask;
    rchkHandleEvent* readEventHandle;
    rchkHandleEvent* writeEventHandle;
    void* clientData;
} RchkEvent;

typedef struct RchkEventLoop {
    int fd;
    int setsize; /* max number of file descriptors tracked */
    RchkEvent* events; /* registered events */
    void* apiData; /* This is used for polling API specific data */
} RchkEventLoop;

RchkEventLoop* rchkEventLoopNew(int setsize);
int  rchkEventLoopRegister(RchkEventLoop* eventLoop, int fd, int mask, rchkHandleEvent* proc, void* clientData);
void rchkEventLoopUnregister(RchkEventLoop* eventLoop, int fd, int mask);
void rchkEventLoopMain(RchkEventLoop* eventLoop); // main event loop
void rchkEventLoopFree(RchkEventLoop* eventLoop);

#endif

