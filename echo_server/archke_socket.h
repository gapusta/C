#ifndef ARCHKE_SOCKET_API
#define ARCHKE_SOCKET_API

#define ARCHKE_SOCKET_MODE_BLOCKING 1
#define ARCHKE_SOCKET_MODE_NON_BLOCKING 2

int  rchkSocketSetMode(int socketFd, int mode);
int  rchkSocketRead(int socketFd, char* buffer, int bufferSize);
int  rchkSocketWrite(int socketFd, char* buffer, int n);
void rchkSocketClose(int socketFd);

int  rchkServerSocketNew(int port);
int  rchkServerSocketAccept(int serverSocketFd);
void rchkServerSocketClose(int serverSocketFd);

#endif

