#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/un.h>
#include <pthread.h>

#include "board_library.h"
#include "connections.h"

int client[2];

void* connection_thread (void* socket_desc);


int main(int argc, char const *argv[]) {
    int server_fd;
    struct sockaddr_in address;
    

    pthread_t thread_id[10];
    int i = 0;

    establish_server_connections(&address, &server_fd);
    init_board(4);

    while(1){
        printf("Waiting for next client\n");
        client[i] = server_accept_client(&address, &server_fd);
        printf("%d\n", client[i]);
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
    card_info card;
    // COR

    while(1){
        recv(client_socket, &board_x, sizeof(board_x), 0);
        recv(client_socket, &board_y, sizeof(board_y), 0);
        resp = board_play(board_x, board_y);

        switch (resp.code) {
            case 1:
                //paint_card(resp.play1[0], resp.play1[1] , 7, 200, 100);
                //write_card(resp.play1[0], resp.play1[1], resp.str_play1, 200, 200, 200);
                // AQUI -> Cordenadas, Cor, string, Cor da string
                // QUANDO ISTO DER ***** PESQUISAR PACKETS !!!!!!!
                card.x = board_x;
                card.y = board_y;
                card.card_color[0] = 107;
                card.card_color[1] = 200;
                card.card_color[2] = 100;
                strcpy(card.card_string, resp.str_play1);
                card.

                for (int i = 0; i < 2; i++)
                    send(client[i], &resp, sizeof(resp), 0);
                break;
            case 3:
                done = 1;
            case 2:
                paint_card(resp.play1[0], resp.play1[1] , 107, 200, 100);
                write_card(resp.play1[0], resp.play1[1], resp.str_play1, 0, 0, 0);
                paint_card(resp.play2[0], resp.play2[1] , 107, 200, 100);
                write_card(resp.play2[0], resp.play2[1], resp.str_play2, 0, 0, 0);
                break;
            case -2:
                paint_card(resp.play1[0], resp.play1[1] , 107, 200, 100);
                write_card(resp.play1[0], resp.play1[1], resp.str_play1, 255, 0, 0);
                paint_card(resp.play2[0], resp.play2[1] , 107, 200, 100);
                write_card(resp.play2[0], resp.play2[1], resp.str_play2, 255, 0, 0);
                sleep(2);
                paint_card(resp.play1[0], resp.play1[1] , 255, 255, 255);
                paint_card(resp.play2[0], resp.play2[1] , 255, 255, 255);
                break;
        }


        recv(client_socket, &board_x, sizeof(board_x), 0);
        recv(client_socket, &board_y, sizeof(board_y), 0);
        resp = board_play(board_x, board_y);
        for (int i = 0; i < 2; i++)
            send(client[i], &resp, sizeof(resp), 0);
    }

}


/*

int main(int argc, char const *argv[]) {
    int server_fd;
    struct sockaddr_in address;
    int client;
    int board_x, board_y;
    int client2;
    play_response resp2; 

    establish_server_connections(&address, &server_fd);

    client = server_accept_client(&address, &server_fd);
    printf("%d\n", client);
    client2 = server_accept_client(&address, &server_fd);
    printf("%d\n", client2);
    init_board(4);

    
    while(1){
        recv(client, &board_x, sizeof(board_x), 0);
        recv(client, &board_y, sizeof(board_y), 0);
        play_response resp = board_play(board_x, board_y);
        send(client, &resp, sizeof(resp), 0);

        recv(client2, &board_x, sizeof(board_x), 0);
        recv(client2, &board_y, sizeof(board_y), 0);
        resp2 = board_play(board_x, board_y);
        send(client2, &resp2, sizeof(resp2), 0);
    }
    

    close(server_fd);
    return 0;
}
*/