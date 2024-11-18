#ifndef ARCHKE_SOCKET_API
#define ARCHKE_SOCKET_API

#define ARCHKE_SOCKET_MODE_BLOCKING 1
#define ARCHKE_SOCKET_MODE_NON_BLOCKING 2

int rchkSocketSetNonBlocking(int socketFd);
void rchkSocketClose(int socketFd);

int rchkServerSocketNew(int port);
int rchkServerSocketAccept(int serverSocketFd);

#endif

