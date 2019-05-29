#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <pthread.h>
#include <poll.h>

#include "board_library.h"
#include "connections.h"

//Locked and blocked
//Mudar quando clicamos numa carta virada
//size = sizeof(struct sockaddr_in)
//sizeof inicializada sempre antes de cada accept
// Quando se criar a lista de clientes, talvez incluir tambem o thread ID correspondente a cada cliente nela
// Adicionar limite de size, na funcao read_arguments


int white[3] = {255,255,255}, black[3] = {0,0,0}, red[3] = {255,0,0}, grey[3]={200,200,200};
// mudar os mutexes para a a board em si, deve ser melhor
void* connection_thread (void* socket_desc);
void Update_Board (board_place *card, int c_color[3], int s_color[3]);
card_info* Allocate_Board_Cards (int dim);
int read_arguments (int argc, char* argv);
void Send_Board (int socket, board_place* board, int dim);
void Copy_Card (board_place board, card_info* card, int board_x, int board_y);
void Check_Winner (void);
int Count_5_seconds ();



int Board_size = 0;

int main(int argc, char const *argv[]) {
    int server_fd;
    struct sockaddr_in address;
    pthread_t thread_id[10];
    int i = 0;

    Board_size = read_arguments(argc, (char*)argv[1]);
    establish_server_connections(&address, &server_fd);
    init_board(Board_size);


    while(1){
        Client_list = Add_Client(server_accept_client(&address, &server_fd));
        pthread_create (&thread_id[i], NULL, connection_thread, (void*)&Client_list->client.client_socket);
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
    card_info card;
    int ret;
    // Codigo das cores precisa de ser melhorado
    int faded_player_color[3]={rand()%205,rand()%255,rand()%255};
    int player_color[3]={faded_player_color[0]+50,faded_player_color[1],faded_player_color[2]};

    //Send current board game when client connects for the first time
    Send_Board (client_socket, board, Board_size);

    while(endgame != 1){
        recv_size = recv(client_socket, &board_x, sizeof(board_x), 0);
        if (recv_size == 0)
            break;
        recv_size = recv(client_socket, &board_y, sizeof(board_y), 0);
        if (recv_size == 0)
            break;  

        resp = board_play(board_x, board_y, play1);
        i = linear_conv (resp.play1[0], resp.play1[1]);
        j = linear_conv (resp.play2[0], resp.play2[1]);

        switch (resp.code) {
            case 1: // first card is played
                Update_Board(&board[i], faded_player_color, grey);
                Copy_Card (board[i], &card, resp.play1[0], resp.play1[1]);
                send_all_clients(card);
                
                ret = Count_5_seconds();

                if (ret == 0){
                    printf("N houve jogada\n");
                    Update_Board(board, white, black);
                    Copy_Card (*board, &card, resp.play1[0], resp.play1[1]);
                    send_all_clients(card);
                    resp.code = -2;
                }
                break;
            case 3: // end of game
                endgame = 1;
                Check_Winner();
            case 2: // cards are a match
                Update_Board(&board[i], player_color,  black);
                Copy_Card (board[i], &card, resp.play1[0], resp.play1[1]);
                send_all_clients(card);

                Update_Board(&board[j], player_color, black);
                Copy_Card (board[j], &card, resp.play2[0], resp.play2[1]);
                send_all_clients(card);  

                break;
            case -2:    // cards are NOT a match
                Update_Board(&board[i], player_color, red);
                Copy_Card (board[i], &card, resp.play1[0], resp.play1[1]);
                send_all_clients(card);

                Update_Board(&board[j], player_color, red);
                Copy_Card (board[j], &card, resp.play2[0], resp.play2[1]);
                send_all_clients(card);

                sleep(2);

                //unlock_board_mutex(board->play1[0], resp.play1[1]);
                pthread_mutex_unlock(&board[linear_conv(resp.play1[0], resp.play1[1])].mutex);
                Update_Board(&board[i], white, black);
                Copy_Card (board[i], &card, resp.play1[0], resp.play1[1]);
                send_all_clients(card);

                //unlock_board_mutex(resp->play2[0], resp.play2[1]);
                pthread_mutex_unlock(&board[linear_conv(resp.play2[0], resp.play2[1])].mutex);
                Update_Board(&board[j], white,  black);
                Copy_Card (board[j], &card, resp.play2[0], resp.play2[1]);
                send_all_clients(card);
                break;
            case -1:    // Turn the card back down
                Update_Board(&board[i], white, black);
                Copy_Card (board[i], &card, resp.play1[0], resp.play1[1]);
                send_all_clients(card);
                break;
        }
    }
    Remove_Client(client_socket); 
    printf("Closing connection_thread\n");
    return 0;
}


void Update_Board (board_place *board, int c_color[3], int s_color[3]) {

    //pthread_mutex_lock (&board->mutex);
    board->card_color[0] = c_color[0];
    board->card_color[1] = c_color[1];
    board->card_color[2] = c_color[2];
    
        board->string_color[0] = s_color[0];
        board->string_color[1] = s_color[1];
        board->string_color[2] = s_color[2];
    //pthread_mutex_unlock (&board->mutex);
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

int read_arguments (int argc, char*argv) {
    int size = 0;

    if (argc != 2){
        printf("Not enough arguments\n");
        exit(EXIT_FAILURE);
    }
    sscanf(argv, "%d", &size);
    if (size <= 0) {
        printf("The size needs to be a positive number\n");
        exit(EXIT_FAILURE);
    }
    else if (size % 2 != 0) {
        printf("The size needs to be an even number\n");
        exit(EXIT_FAILURE);   
    }
    else if (size > 36) {   // (26 letras * 26 letras * 2 cartas)^(1/2) é a dimensao maxima
        printf("The maximum size is 36\n");
        exit(EXIT_FAILURE);      
    }
    // ADICIONAR LIMITE AQUI !!!!
    return size; 
}

void Send_Board (int socket, board_place* board, int dim){
    int i;
    int x;
    int y;
    card_info card;

    write(socket,&dim, sizeof(dim));
    char* str = malloc(sizeof(card_info));

    for (i = 0; i< dim*dim; i++){
        if (board[i].card_color[0] == 255 && board[i].card_color[1] == 255 && board[i].card_color[2] == 255){
            continue;
        }
        inverse_linear_conv (i, &x, &y);
        Copy_Card (board[i], &card, x, y);
        memcpy(str, &card, sizeof(card_info));
        write(socket,str, sizeof(card_info));
    }
}

void Copy_Card (board_place board, card_info* card, int board_x, int board_y){

    card->x = board_x;
    card->y = board_y;
    card->card_color[0] = board.card_color[0];
    card->card_color[1] = board.card_color[1];
    card->card_color[2] = board.card_color[2];
    strcpy(card->string, board.v);
    card->string_color[0] = board.string_color[0];
    card->string_color[1] = board.string_color[1];
    card->string_color[2] = board.string_color[2];
}

void Check_Winner (void){
    Node* aux = Client_list;
    int winner = 0;
    int score = 0;

    while(aux != NULL){
        if (aux->client.score > score){
            score = aux->client.score;
            winner = aux->client.client_socket;
        }
        aux = aux->next;
    }

    printf("The winner is player number %d with %d points\n", winner, score);
}

int Count_5_seconds (){
    struct pollfd fds[1];
    int timeout;
    int fd = 0;
    int ret;
    card_info card;

    while(1){
        fds[0].fd = fd;
        fds[0].events = 0;
        fds[0].events |= POLLIN;

        timeout = 1000;
        ret = poll(fds,1,timeout);
        return ret;
    }
}
                