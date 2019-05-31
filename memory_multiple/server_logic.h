#ifndef _SERVER_LOGIC_H_
#define _SERVER_LOGIC_H_ 

#include "board_library.h"
#include "connections.h"

#include <stdio.h>
#include <string.h>
#include <poll.h>
#include <stdlib.h>
#include <pthread.h>

int read_arguments (int argc, char* argv);
void Copy_Card (board_place board, card_info* card, int board_x, int board_y);
int poll_x_milliseconds (int client_socket, int timeout);
void count_x_seconds_ignore_recv (int client_socket, int timeout);
void Send_Board (int socket);
void save_and_send_card (int player_color[3], int letter_color[3], int x, int y);
void reset_board_and_update_all_clients ();
void Check_Winner (int player_socket);
int Validate_Message(int position);
void Release_Resources(void);

#endif