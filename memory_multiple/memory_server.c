#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/un.h>
#include <pthread.h>

#include "board_library.h"
#include "connections.h"

//Locked and blocked
//Mudar quando clicamos numa carta virada
//size = sizeof(struct sockaddr_in)
//sizeof inicializada sempre antes de cada accept
// Quando se criar a lista de clientes, talvez incluir tambem o thread ID correspondente a cada cliente nela

int white[3] = {255,255,255}, black[3] = {0,0,0}, red[3] = {255,0,0}, grey[3]={200,200,200};

void* connection_thread (void* socket_desc);
void* first_card_timeout (void* coords);
void Update_Board (card_info *card, int x, int y, int c_color[3], char* str, int s_color[3], int status);
void send_all_clients (card_info card);
Node * Add_Client (int new_client);
Node * Remove_Client (int client);
void Print_List ();
void Allocate_Mutexes_Array (int dim, pthread_mutex_t** mux_a);
card_info* Allocate_Board_Cards (int dim);
void Send_Board (int socket, card_info* board, int dim);
int Convert_Coordinates (int x, int y, int dim);

pthread_mutex_t **mutexes;

Node * Client_list;

card_info* Board_cards;

int Board_size = 0;

int main(int argc, char const *argv[]) {
    int server_fd;
    struct sockaddr_in address;
    pthread_t thread_id[10];
    int i = 0;

    //Board_size = atoi (argv[1]);
    Board_size = 4;
    Allocate_Mutexes_Array (Board_size, mutexes);
    Board_cards = Allocate_Board_Cards (Board_size);

    establish_server_connections(&address, &server_fd);
    init_board(4);

    while(1){
        Client_list = Add_Client(server_accept_client(&address, &server_fd));
        pthread_create (&thread_id[i], NULL, connection_thread, (void*)&Client_list->client_socket);
        i++;
    }

    close(server_fd);
    return 0;
}

//Criar a cor fora thread para garantir que é única

void* connection_thread (void* socket_desc){
    int board_x, board_y;
    play_response resp;
    int client_socket = *(int*)socket_desc;
    int play1[2] = {-1, 0};
    int recv_size;
    int i,j;
    int endgame;
    // Codigo das cores precisa de ser melhorado
    int faded_player_color[3]={rand()%205,rand()%255,rand()%255};
    int player_color[3]={faded_player_color[0]+50,faded_player_color[1],faded_player_color[2]};
    pthread_t timeout_thread_id;


    //Send current board game when client connects for the first time
    Send_Board (client_socket, Board_cards, Board_size);

    while(endgame != 1){
        recv_size = recv(client_socket, &board_x, sizeof(board_x), 0);
        if (recv_size == 0)
            break;
        recv_size = recv(client_socket, &board_y, sizeof(board_y), 0);
        if (recv_size == 0)
            break;  

        resp = board_play(board_x, board_y, play1);
        i = Convert_Coordinates (resp.play1[0], resp.play1[1], Board_size);
        j = Convert_Coordinates (resp.play2[0], resp.play2[1], Board_size);

        switch (resp.code) {
            case 1:
                Update_Board(&Board_cards[i], resp.play1[0], resp.play1[1], faded_player_color, resp.str_play1, grey, 1);
                send_all_clients(Board_cards[i]);
                // Thread to change the card in case it times out
                pthread_create (&timeout_thread_id, NULL, first_card_timeout, (void*)&resp.play1);
                break;
            case 3:
                endgame = 1;
            case 2:
                Update_Board(&Board_cards[i], resp.play1[0], resp.play1[1], player_color, resp.str_play1, black, 1);
                send_all_clients(Board_cards[i]);

                Update_Board(&Board_cards[j], resp.play2[0], resp.play2[1], player_color, resp.str_play2, black, 1);
                send_all_clients(Board_cards[j]);    
                break;
            case -2:
                Update_Board(&Board_cards[i], resp.play1[0], resp.play1[1], player_color, resp.str_play1, red, 1);
                send_all_clients(Board_cards[i]);

                Update_Board(&Board_cards[j], resp.play2[0], resp.play2[1], player_color, resp.str_play2, red, 1);
                send_all_clients(Board_cards[j]);

                sleep(2);

                set_card_state(resp.play1[0], resp.play1[1], DOWN);
                Update_Board(&Board_cards[i], resp.play1[0], resp.play1[1], white, NULL, black, 0);
                send_all_clients(Board_cards[i]);

                set_card_state(resp.play2[0], resp.play2[1], DOWN);
                Update_Board(&Board_cards[j], resp.play2[0], resp.play2[1], white, NULL, black, 0);
                send_all_clients(Board_cards[j]);
                break;
        }
    }
    Remove_Client(client_socket); 
    printf("Closing connection_thread\n");
    return 0;
}

void* first_card_timeout (void* arg) {
    int coords[2];
    card_info card;
    coords[0] = ((int*)arg)[0];
    coords[1] = ((int*)arg)[1];
    int aux;
    int card_changed = 0;

    // wait 5 seconds, and check if the card hasn't changed
    for (aux = 0; aux < 5; aux++) {
        sleep(1);   // If the play is incorrect, it blocks for 2 secs, so this will not fail
        if (get_card_state(coords[0],coords[1]) != UP){
            card_changed = 1;
            break;
        }
    }
    if (card_changed == 0) {
        set_card_state(coords[0], coords[1], DOWN);
        Update_Board(&card, coords[0], coords[1], white, NULL, black, 0);
        send_all_clients(card);
    }
    return 0;
}

// Perguntar a laisa se faz sentido mandar esta funcao para o connections .c
void Update_Board (card_info *card, int x, int y, int c_color[3], char* str, int s_color[3], int status) {

    card->x = x;
    card->y = y;
    card->card_color[0] = c_color[0];
    card->card_color[1] = c_color[1];
    card->card_color[2] = c_color[2];
    if (str != NULL) {
        strcpy(card->string, str);
        card->string_color[0] = s_color[0];
        card->string_color[1] = s_color[1];
        card->string_color[2] = s_color[2];
    }
    card->state = status;
}

void send_all_clients (card_info card) {
    Node* aux = Client_list;

    char* str = malloc(sizeof(card_info));
    memcpy(str, &card, sizeof(card_info));

    while(aux != NULL){

        write(aux->client_socket,str, sizeof(card_info));
        //send(aux->client_socket, &card, sizeof(card), 0);
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
    

void Print_List (){
    if (Client_list == NULL)
        return;

    Node* aux = Client_list;

    while(aux != NULL){
        printf("number is %d\n", aux->client_socket);
        aux = aux->next;
    }
}


void Allocate_Mutexes_Array (int dim, pthread_mutex_t** mux_a){

    mux_a = malloc (sizeof(pthread_mutex_t*)*dim);

    for (int i = 0; i < dim; i++)
    {
        mux_a[i] = malloc (sizeof (pthread_mutex_t));
    }
}

card_info* Allocate_Board_Cards (int dim){

    int i;

    card_info* array = malloc (sizeof(card_info)*dim*dim);

    for (i=0; i < dim*dim; i++){
        array[i].x = -1;
        array[i].y = -1;
        array[i].card_color[0] = 255;
        array[i].card_color[1] = 255;
        array[i].card_color[2] = 255;
        strcpy (array[i].string, "\0");
        array[i].string_color[0] = 255;
        array[i].string_color[1] = 255;
        array[i].string_color[2] = 255;
        array[i].end = 0;
        array[i].state = 0;
    }
    return array;
}

void Send_Board (int socket, card_info* board, int dim){

    int i;
    char* str = malloc(sizeof(card_info));

    for (i = 0; i< dim*dim; i++){
        if (board[i].state == 1){
            memcpy(str, &board[i], sizeof(card_info));
            write(socket,str, sizeof(card_info));
        }
    }
}

int Convert_Coordinates (int x, int y, int dim){
    return x*dim + y;
}

