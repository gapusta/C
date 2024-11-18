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

void rchkSocketClose(int socketFd) {
    shutdown(socketFd, SHUT_WR);
    close(socketFd);
}

int rchkServerSocketNew(int port) {
    // create server socket
	printf("Creating server socket\n");
	int serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocketFd < 0) {
		return -1;
	}

	// bind server socket to address/port
	printf("bind socket to port 9999\n");
	struct sockaddr_in server_address = { 0 };

	server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);
    
    if (bind(serverSocketFd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
		close(serverSocketFd);
        return -1;
	}

	// mark server as "listener" (accept incoming connection requests)
	printf("make server socket listen to port %d\n", port);
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

	printf("Accepted new connection/client : %d\n", clientSocketFd);
	
	rchkSocketSetMode(clientSocketFd, ARCHKE_SOCKET_MODE_NON_BLOCKING);
	printf("Set %d as non-blocking\n", clientSocketFd);

    return clientSocketFd;
}

