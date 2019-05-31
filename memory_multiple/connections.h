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
	int card_color[3];	// R G B
	char string[3];
	int string_color[3];
	int winner_score;
} card_info;

typedef struct {
	int client_socket;
	int score;
	int color[3];
} player_info;

typedef struct node{
	player_info client;
 	struct node *next;
}Node; 

int sock_fd;	// Socket to communicate
Node * Client_list;	// list that contains all the clients

void establish_client_connections (struct sockaddr_in *server_addr, struct sockaddr_in *addr);
void establish_server_connections ( struct sockaddr_in *address, int *server_fd);
int server_accept_client (struct sockaddr_in *address, int *server_fd, int number_of_clients);
void send_all_clients (card_info card);
void Add_Client (int new_client, int *number_of_clients);
void Remove_Client (int client, int *number_of_clients);
#endif