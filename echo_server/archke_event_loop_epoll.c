#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <unistd.h>
#include <sys/epoll.h>
#include "archke_event_loop.h"

RchkEventLoop* rchkEventLoopNew(int setsize) {
    RchkEventLoop* eventLoop = malloc(sizeof(RchkEventLoop));
    if (eventLoop == NULL) {
        return NULL;
    }

    eventLoop->events = NULL;
    eventLoop->apiData = NULL;

    eventLoop->events = malloc(setsize * sizeof(RchkEvent));
    if (eventLoop->events == NULL) {
        goto err;
    }

    eventLoop->apiData = malloc(setsize * sizeof(struct epoll_event));
    if (eventLoop->apiData == NULL) {
        goto err;
    }

    int epollFd = epoll_create1(0);
  	if (epollFd < 0) {
        goto err;
  	}

    eventLoop->fd = epollFd;
    eventLoop->setsize = setsize;
    
    for (int i=0; i<setsize; i++) {
        eventLoop->events[i].mask = ARCHKE_EVENT_LOOP_NONE_EVENT;
        eventLoop->events[i].readEventHandle = NULL;
        eventLoop->events[i].writeEventHandle = NULL;
        eventLoop->events[i].clientData = NULL;
    }

    return eventLoop;

err:
    free(eventLoop->apiData);
    free(eventLoop->events);
    free(eventLoop);

    return NULL;
}

int rchkEventLoopRegister(RchkEventLoop* eventLoop, int fd, int mask, rchkHandleEvent* proc, void* clientData) {
    // 1. init epoll
    int epollMask = 0;
    if (mask & ARCHKE_EVENT_LOOP_READ_EVENT) epollMask |= EPOLLIN;
    if (mask & ARCHKE_EVENT_LOOP_WRITE_EVENT) epollMask |= EPOLLOUT;
    
    struct epoll_event epollEvent = { 
        .events = epollMask, 
        .data.fd = fd 
    }; 

    int op = eventLoop->events[fd].mask == ARCHKE_EVENT_LOOP_NONE_EVENT ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
    int result = epoll_ctl(eventLoop->fd, op, fd, &epollEvent);
	
    if (result < 0) { 
        return -1; 
    }

    // 2. init additional event information
    RchkEvent* event = &eventLoop->events[fd];
    event->mask = mask;
    if (mask & ARCHKE_EVENT_LOOP_READ_EVENT) { event->readEventHandle = proc; }
    if (mask & ARCHKE_EVENT_LOOP_WRITE_EVENT) { event->writeEventHandle = proc; }
    event->clientData = clientData;

    return 0;
}

void rchkEventLoopUnregister(RchkEventLoop* eventLoop, int fd) {
    epoll_ctl(eventLoop->fd, EPOLL_CTL_DEL, fd, NULL);

    RchkEvent* event = &eventLoop->events[fd];
    event->mask = ARCHKE_EVENT_LOOP_NONE_EVENT;
    event->readEventHandle = NULL;
    event->writeEventHandle = NULL;
    event->clientData = NULL;
}

int rchkEventLoopMain(RchkEventLoop* eventLoop) {
    struct epoll_event* epollEvents = (struct epoll_event*) eventLoop->apiData;
	for(;;) {
		int nevents = epoll_wait(eventLoop->fd, epollEvents, eventLoop->setsize, -1);
        if (nevents < 0) {
            return -1;
        }

        for (int i=0; i<nevents; i++) {
            int fd = epollEvents[i].data.fd;
            RchkEvent* event = &eventLoop->events[fd];

            if (epollEvents[i].events & EPOLLIN) {
                event->readEventHandle(eventLoop, fd, event, event->clientData);
            }

            if (epollEvents[i].events & EPOLLOUT) {
                event->writeEventHandle(eventLoop, fd, event, event->clientData);
            }
        }
    }

    return 0;
}

void rchkEventLoopFree(RchkEventLoop* eventLoop) {
    close(eventLoop->fd);
    free(eventLoop->apiData);
    free(eventLoop->events); // TODO: How to free 'clientData'?
    free(eventLoop);
}

