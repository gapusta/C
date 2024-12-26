#ifndef ARCHKE_SOCKET_API
#define ARCHKE_SOCKET_API

#define ARCHKE_SOCKET_MODE_BLOCKING 1
#define ARCHKE_SOCKET_MODE_NON_BLOCKING 2

typedef struct RchkSocketBuffer {
    char* buffer;
    int size;
} RchkSocketBuffer;

int  rchkSocketSetMode(int socketFd, int mode);
int  rchkSocketRead(int socketFd, char* buffer, int bufferSize);
int  rchkSocketWrite(int socketFd, char* buffer, int n);
int  rchkSocketWritev(int socketFd, RchkSocketBuffer* buffers, int n);
void rchkSocketShutdownWrite(int socketFd);
void rchkSocketShutdownRead(int socketFd);
void rchkSocketShutdown(int socketFd);
void rchkSocketClose(int socketFd);

int  rchkServerSocketNew(int port);
int  rchkServerSocketAccept(int serverSocketFd);
void rchkServerSocketClose(int serverSocketFd);

#endif

