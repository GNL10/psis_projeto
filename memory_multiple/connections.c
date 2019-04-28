#include "connections.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void establish_client_connections (struct sockaddr_in *server_addr, struct sockaddr_in *addr) {
	int error;

	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd == -1){
		perror("socket: ");
		exit(-1);
	}

	memset(server_addr, '0', sizeof(*server_addr));
	addr->sin_family = AF_INET;
	server_addr->sin_port = htons(PORT);

	if(inet_pton(AF_INET, "127.0.0.1", &server_addr->sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        exit (-1); 
    } 

	server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(PORT); 
 	error = connect(sock_fd, (struct sockaddr *)server_addr, sizeof(*server_addr));
	if(error == -1) {
		perror("connect");
		exit(-1);
	}
}

void establish_server_connections ( struct sockaddr_in *address, int *server_fd) {
	*server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (*server_fd == -1){
		perror("socket: ");
		exit(-1);
	}

	address->sin_family = AF_INET;
	address->sin_addr.s_addr = INADDR_ANY; 
    address->sin_port = htons( PORT );
    if (bind(*server_fd, (struct sockaddr *)address,  
                                 sizeof(*address))<0) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    }
    if (listen(*server_fd, 3) < 0)	// segundo argumento de listen é o limite da fila de conexoes 
    { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    }
}

int server_accept_client (struct sockaddr_in *address, int *server_fd) {
    int new_socket;
    int addrlen = sizeof(*address);

    if ((new_socket = accept(*server_fd, (struct sockaddr *)&address,  
                       (socklen_t*)&addrlen))<0) { 
        perror("accept"); 
        exit(EXIT_FAILURE); 
    }

    return new_socket;
}