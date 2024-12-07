#include <sys/socket.h> 
#include <netinet/in.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "archke_socket.h"

int rchkSocketSetMode(int socketFd, int mode) {
    // get socket's flags
    int flags = fcntl(socketFd, F_GETFL, 0);
	if (flags < 0) {
		return -1;    
  	}
    // set "is nonblocking" flag
  	if (fcntl(socketFd, F_SETFL, flags | O_NONBLOCK) < 0) {
    	return -1;
  	}

    return 0;
}

int rchkSocketRead(int socketFd, char* buffer, int bufferSize) {
	ssize_t nbytes = read(socketFd, buffer, bufferSize);
	if (nbytes == -1) {
		return -1;
	}

	return nbytes;
}

int rchkSocketWrite(int socketFd, char* buffer, int n) {
	ssize_t nbytes = write(socketFd, buffer, n);
	if (nbytes < 0) {
		return -1;
	}

	return nbytes;
}

void rchkSocketShutdownWrite(int socketFd) {
	shutdown(socketFd, SHUT_WR);
}

void rchkSocketShutdownRead(int socketFd) {
	shutdown(socketFd, SHUT_RD);
}

void rchkSocketShutdown(int socketFd) {
	shutdown(socketFd, SHUT_RDWR);
}

void rchkSocketClose(int socketFd) {
    close(socketFd);
}

int rchkServerSocketNew(int port) {
    // create server socket
	int serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocketFd < 0) {
		return -1;
	}

	// bind server socket to address/port
	struct sockaddr_in server_address = { 0 };

	server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);
    
    if (bind(serverSocketFd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
		close(serverSocketFd);
        return -1;
	}

	// mark server as "listener" (accept incoming connection requests)
	if (listen(serverSocketFd, SOMAXCONN) < 0) { 
		close(serverSocketFd);
        return -1;
	}

    return serverSocketFd;
}

int rchkServerSocketAccept(int serverSocketFd) {
    struct sockaddr clientAddress = { 0 };
	socklen_t clientAddrLen = sizeof(clientAddress);
	
    int clientSocketFd = accept(serverSocketFd, &clientAddress, &clientAddrLen);
	if (clientSocketFd < 0) {
		return -1;
	}
	
    return clientSocketFd;
}

void rchkServerSocketClose(int serverSocketFd) {
	close(serverSocketFd);
}

