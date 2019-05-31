#ifndef _CONNECTIONS_H_
#define _CONNECTIONS_H_

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#define PORT 3000
#define MAX_CLIENTS 20

typedef struct {
	int x;
	int y;
	int card_color[3];	// RGB -> for the color of the card
	char string[3];		// string with 2 letters
	int string_color[3];// RGB -> for the color of the string
	int winner_score;	// score of the winner at the end of the game
} card_info;

typedef struct {
	int client_socket;	// socket number of the client
	int score;			// current score of the client
	int color[3];		// color assigned to the client
} player_info;

typedef struct node{
	player_info client;	// information about client
 	struct node *next;
}Node; 

void establish_client_connections (struct sockaddr_in *server_addr, struct sockaddr_in *addr);
void establish_server_connections ( struct sockaddr_in *address, int *server_fd);
int server_accept_client (struct sockaddr_in *address, int *server_fd, int number_of_clients);
void send_all_clients (card_info card);
int Add_Client (int new_client, int *number_of_clients);
void Remove_Client (Node* client_out, int *number_of_clients);
void Check_Best_Score(Node *client_out);
#endif