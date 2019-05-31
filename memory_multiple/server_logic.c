#include "server_logic.h"

extern board_place * BOARD; // from board_library.c
extern int BOARD_SIZE;      // from board_library.c

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
    else if (size > 12) {   
        printf("The maximum size is 12\n");
        exit(EXIT_FAILURE);      
    }
    return size; 
}

void Send_Board (int socket, int dim){
    int i;
    int x;
    int y;
    card_info card;

    write(socket,&dim, sizeof(dim));
    char* str = malloc(sizeof(card_info));
    if (str == NULL){
      printf("Allocation error\n");
      exit(EXIT_FAILURE);
    }

    for (i = 0; i< dim*dim; i++){
        if (BOARD[i].card_color[0] == 255 && BOARD[i].card_color[1] == 255 && BOARD[i].card_color[2] == 255){
            continue;
        }
        inverse_linear_conv (i, &x, &y);
        Copy_Card (BOARD[i], &card, x, y);
        memcpy(str, &card, sizeof(card_info));
        write(socket,str, sizeof(card_info));
    }
    free(str);
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
    card->winner_score = 0;
}

void Check_Winner (int player_socket){
    Node* aux = Client_list;
    int winner = 0;
    int score = 0;
    card_info winner_card;

    while(aux != NULL){
        if (aux->client.score > score){
            score = aux->client.score;
            winner = aux->client.client_socket;
        }
        aux = aux->next;
    }

    printf("The winner is player number %d with %d points\n", winner, score);


    if (winner == player_socket){
        char* str = malloc(sizeof(card_info));
        winner_card.winner_score = score;
        strcpy(winner_card.string, "\0");
        memcpy(str, &winner_card, sizeof(card_info));
        write(player_socket,str, sizeof(card_info));
        free(str);
    }

}

int poll_x_milliseconds (int client_socket, int timeout) {
    struct pollfd fds;
    int nfds = 1;
    int ret;

    memset(&fds, 0, sizeof(fds));
    fds.fd = client_socket;
    fds.events = POLLIN;

    ret = poll(&fds, nfds, timeout);
    return ret;
}

/* function count_x_seconds_ignore_recv
    Sleeps for x seconds and polls the socket to know if there is information to be read
    If so, it cleans the pipe, otherwise, it returns
*/
void count_x_seconds_ignore_recv (int client_socket, int sleep_time) {
    char trash[10000];
    int output = 0;

    sleep(sleep_time);
    output = poll_x_milliseconds(client_socket, 0);
    if (output < 0) {
        perror("Poll failed:");
        exit(EXIT_FAILURE);
    }
    else if (output == 0) { // no data to read
        return;
    }
    else {  // there is data to read
        recv(client_socket, trash, sizeof(trash), 0);   // clean the socket
    }
    return;
}

void save_and_send_card (int player_color[3], int letter_color[3], int x, int y) {
    card_info card;
    int i;

    i = linear_conv (x, y);
    Update_Board(&BOARD[i], player_color,  letter_color);
    Copy_Card (BOARD[i], &card, x, y);
    send_all_clients(card);
}

void reset_board_and_update_all_clients () {
    card_info card;
    int x;

    free(BOARD);
    init_board();
    for (x = 0; x < BOARD_SIZE*BOARD_SIZE; x++) {
        Copy_Card (BOARD[x], &card, x%BOARD_SIZE, x/BOARD_SIZE);
        send_all_clients(card);
    }
}