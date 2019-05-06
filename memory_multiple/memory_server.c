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
void assign_card_parameters (card_info *card, int x, int y, int card_R, int card_G, int card_B, char* str, int str_R, int str_B, int str_G);
void send_all_clients (card_info card);
Node * Add_Client (int new_client);
Node * Remove_Client (int client);

Node * Client_list;

int main(int argc, char const *argv[]) {
    int server_fd;
    struct sockaddr_in address;
    pthread_t thread_id[10];
    int i = 0;
    int n = 0;


    //size = sizeof(struct sockaddr_in)
    //sizeof inicializada sempre antes de cada accept

    establish_server_connections(&address, &server_fd);
    init_board(4);

    while(1){
        Client_list = Add_Client(server_accept_client(&address, &server_fd));
        pthread_create (&thread_id[i], NULL, connection_thread, (void*)&Client_list->client_socket);
    }
    
    /*
    establish_server_connections(&address, &server_fd);
    init_board(4);

    while(1){
        client[i] = server_accept_client(&address, &server_fd);
        pthread_create (&thread_id[i], NULL, connection_thread, (void*)&client[i]);
        i++;
    }*/

    close(server_fd);
    return 0;
}

void* connection_thread (void* socket_desc){
    int board_x, board_y;
    play_response resp;
    int client_socket = *(int*)socket_desc;
    card_info card;
    card.end = 0;
    int play1[2];
    char saved_first_string[3];

    play1[0] = -1;
    // COR

    while(card.end != 1){
        recv(client_socket, &board_x, sizeof(board_x), 0);
        recv(client_socket, &board_y, sizeof(board_y), 0);
        resp = board_play(board_x, board_y, play1, saved_first_string);

        switch (resp.code) {
            case 1:
                // PESQUISAR PACKETS !!!!!!!
                assign_card_parameters(&card, resp.play1[0], resp.play1[1], 7, 200, 100, resp.str_play1, 200, 200, 200);
                send_all_clients(card);
                break;
            case 3:
                card.end = 1;
            case 2:
                assign_card_parameters(&card, resp.play1[0], resp.play1[1], 107, 200, 100, resp.str_play1, 0, 0, 0);
                send_all_clients(card);

                assign_card_parameters(&card, resp.play2[0], resp.play2[1], 107, 200, 100, resp.str_play2, 0, 0, 0);
                send_all_clients(card);    
                break;
            case -2:
                assign_card_parameters(&card, resp.play1[0], resp.play1[1], 107, 200, 100, resp.str_play1, 255, 0, 0);
                send_all_clients(card);

                assign_card_parameters(&card, resp.play2[0], resp.play2[1], 107, 200, 100, resp.str_play2, 255, 0, 0);
                send_all_clients(card);

                sleep(2);
                
                assign_card_parameters(&card, resp.play1[0], resp.play1[1], 255, 255, 255, NULL, 0, 0, 0);
                send_all_clients(card);

                assign_card_parameters(&card, resp.play2[0], resp.play2[1], 255, 255, 255, NULL, 0, 0, 0);
                send_all_clients(card);
                break;
        }
    }
    printf("Closing server\n");
    return 0;
}

void assign_card_parameters (card_info *card, int x, int y, int card_R, int card_G, int card_B, char* str, int str_R, int str_B, int str_G) {
    card->x = x;
    card->y = y;
    card->card_color[0] = card_R;
    card->card_color[1] = card_G;
    card->card_color[2] = card_B;
    if (str != NULL) {
        strcpy(card->string, str);
        card->string_color[0] = str_R;
        card->string_color[1] = str_G;
        card->string_color[2] = str_B;
    }
}

void send_all_clients (card_info card) {
    Node* aux = Client_list;

    while(aux != NULL){
        send(aux->client_socket, &card, sizeof(card), 0);
        aux = aux->next;
    }
}   

Node * Add_Client (int new_client){
    Node* new_node = NULL;

    new_node = malloc (sizeof(Node));
    if (new_node == NULL){
        printf("Erro de alocação\n");
        exit (EXIT_FAILURE);
    }

    new_node->client_socket = new_client;
    new_node->next = NULL;

    if (Client_list == NULL)
        Client_list = new_node;
    else{
        new_node->next = Client_list;
        Client_list = new_node;
    } 
    return Client_list;
}

Node * Remove_Client (int client){
    Node* aux = Client_list;
    Node* aux2 = Client_list;
    if (Client_list == NULL)
        return Client_list; //ver
    aux = Client_list;
    aux2 = Client_list->next;

    while (aux2 != NULL && aux2->client_socket != client){
        aux = aux2;
        aux2 = aux2->next;
    }
    if (aux2 != NULL){
        aux->next = aux2->next;
        free (aux2);
    }

    return Client_list;
}