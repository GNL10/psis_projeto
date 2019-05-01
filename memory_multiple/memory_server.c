#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/un.h>
#include <pthread.h>

#include "board_library.h"
#include "connections.h"


void* connection_thread (void* socket_desc);


int main(int argc, char const *argv[]) {
    int server_fd;
    struct sockaddr_in address;
    int client[10];

    pthread_t thread_id[10];
    int i = 0;

    establish_server_connections(&address, &server_fd);

    while(1){
        client[i] = server_accept_client(&address, &server_fd);
        printf("%d\n", client[i]);
        printf("%d\n", &client[i]);
        pthread_create (&thread_id[i], NULL, connection_thread, (void*)&client[i]);
        i++;
    }

   
    close(server_fd);
    return 0;
}

void* connection_thread (void* socket_desc){
    int board_x, board_y;
    play_response resp;
    int client_socket = *(int*)socket_desc;

    while(1){
        recv(client_socket, &board_x, sizeof(board_x), 0);
        recv(client_socket, &board_y, sizeof(board_y), 0);
        resp = board_play(board_x, board_y);
        send(client_socket, &resp, sizeof(resp), 0);
    }

}