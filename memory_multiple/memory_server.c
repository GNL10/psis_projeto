#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/un.h>

#include "board_library.h"
#include "connections.h"



int main(int argc, char const *argv[]) {
    int server_fd;
    struct sockaddr_in address;
    int client;
    int board_x, board_y;

    establish_server_connections(&address, &server_fd);

    client = server_accept_client(&address, &server_fd);
    
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