#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/un.h>

#include "board_library.h"

#define PORT 8080

void establish_connections ( struct sockaddr_in *address, int *server_fd) {
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

int connect_client (struct sockaddr_in *address, int *server_fd) {
    int new_socket;
    int addrlen = sizeof(*address);

    if ((new_socket = accept(*server_fd, (struct sockaddr *)&address,  
                       (socklen_t*)&addrlen))<0) { 
        perror("accept"); 
        exit(EXIT_FAILURE); 
    }

    return new_socket;
}


int main(int argc, char const *argv[]) {
    int server_fd;
    struct sockaddr_in address;
    int client;
    int board_x, board_y;

    establish_connections(&address, &server_fd);

    client = connect_client(&address, &server_fd);
    
    init_board(4);

    while (1) {
        recv(client, &board_x, sizeof(board_x), 0);
        recv(client, &board_y, sizeof(board_y), 0);
        play_response resp = board_play(board_x, board_y);
        send(client, &resp, sizeof(resp), 0);
    }

    close(server_fd);
    return 0;
}