#include "board_library.h"
#include "connections.h"
#include "server_logic.h"

extern board_place * BOARD; // from board_library.c
extern int BOARD_SIZE;      // from board_library.c

int NUMBER_OF_CLIENTS = 0;  // current number of clients connected to the server
// Standard colors
int WHITE[3] = {255,255,255}, BLACK[3] = {0,0,0}, RED[3] = {255,0,0}, GREY[3]={200,200,200}, NO_COLOR[3]={-1,-1,-1};

void* connection_thread (void* socket_desc);

int main(int argc, char const *argv[]) {
    int server_fd;
    struct sockaddr_in address;
    pthread_t thread_id[10];
    int i = 0;

    BOARD_SIZE = read_arguments(argc, (char*)argv[1]);
    establish_server_connections(&address, &server_fd);
    init_board();

    // Accepts clients continuously
    while(1){
        Add_Client(server_accept_client(&address, &server_fd), &NUMBER_OF_CLIENTS);
        pthread_create (&thread_id[i], NULL, connection_thread, (void*)Client_list);
        i++;
    }
    printf("CLOSING SERVER\n");
    close(server_fd);
    return 0;
}

//Criar a cor fora thread para garantir que é única

void* connection_thread (void* socket_desc){
    int board_x, board_y;
    play_response resp;
    Node * current_client = (Node*)socket_desc;
    int play1[2] = {-1, 0};
    int endgame = 0;
    int client_connected = 1;
    // Codigo das cores precisa de ser melhorado
    int faded_player_color[3]={rand()%205,rand()%255,rand()%255};
    int player_color[3]={faded_player_color[0]+50,faded_player_color[1],faded_player_color[2]};

    //Send current board game when client connects for the first time
    Send_Board (current_client->client.client_socket);

    while (client_connected == 1) {
        while(endgame != 1){

            if (recv(current_client->client.client_socket, &board_x, sizeof(board_x), 0) == 0){
                client_connected = 0;
                break;
            }
            if (recv(current_client->client.client_socket, &board_y, sizeof(board_y), 0) == 0){
                client_connected = 0;
                break;
            }
            if (NUMBER_OF_CLIENTS < 2)  // waits for at least 2 players
                continue;

            resp = board_play(board_x, board_y, play1);

            switch (resp.code) {
                case 1:     // first card is played
                    save_and_send_card(faded_player_color, GREY, resp.play1[0], resp.play1[1]);

                    // if the poll timed out, turn card back down
                    if (poll_x_milliseconds(current_client->client.client_socket, 5000) == 0){
                        play1[0] = -1;  // reset play1, for the board_play to default to the first play 
                        save_and_send_card(WHITE, NO_COLOR, resp.play1[0], resp.play1[1]);
                        pthread_mutex_unlock(&BOARD[linear_conv(resp.play1[0], resp.play1[1])].mutex);
                    }
                    break;
                case -1:    // first card is turned back down
                    save_and_send_card(WHITE, NO_COLOR, resp.play1[0], resp.play1[1]);
                    pthread_mutex_unlock(&BOARD[linear_conv(resp.play1[0], resp.play1[1])].mutex);
                    break;
                case 2:     // cards are a match
                    // paint both cards with black letters (match)
                    save_and_send_card(player_color, BLACK, resp.play1[0], resp.play1[1]);
                    save_and_send_card(player_color, BLACK, resp.play2[0], resp.play2[1]);
                    current_client->client.score++;
                    break;
                case -2:    // cards are not a match
                    // display cards with red letters
                    save_and_send_card(player_color, RED, resp.play1[0], resp.play1[1]);
                    save_and_send_card(player_color, RED, resp.play2[0], resp.play2[1]);
                    // wait 2 seconds and ignore de recvs
                    count_x_seconds_ignore_recv (current_client->client.client_socket, 2);
                    
                    // after the 2 seconds, the cards go back to white (and their mutexes are unlocked)
                    save_and_send_card(WHITE, NO_COLOR, resp.play1[0], resp.play1[1]);
                    pthread_mutex_unlock(&BOARD[linear_conv(resp.play1[0], resp.play1[1])].mutex);
                    save_and_send_card(WHITE, NO_COLOR, resp.play2[0], resp.play2[1]);
                    pthread_mutex_unlock(&BOARD[linear_conv(resp.play2[0], resp.play2[1])].mutex);
                    break;
                case 3:     // end of game
                    // paint both cards with black letters (match)
                    save_and_send_card(player_color, BLACK, resp.play1[0], resp.play1[1]);
                    save_and_send_card(player_color, BLACK, resp.play2[0], resp.play2[1]);

                    endgame = 1;
                    current_client->client.score++;
                    Check_Winner(current_client->client.client_socket);
                    break;
                default:
                    continue;
            }
        }
        if (endgame == 1) {
            printf("Game ended, waiting 10 seconds!\n");
            count_x_seconds_ignore_recv(current_client->client.client_socket, 10);
            // new board and then send all cards back to white, to all the clients
            reset_board_and_update_all_clients();
            endgame = 0;
        }
    }
    Remove_Client(current_client->client.client_socket, &NUMBER_OF_CLIENTS); 
    printf("Closing connection_thread\n");
    pthread_exit(0);
}