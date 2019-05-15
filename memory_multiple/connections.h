#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
// colocar estas bibliotecas no .c

#define PORT 3000

typedef struct {
	int x;
	int y;
	int card_color[3];	// R G B
	char string[3];
	int string_color[3];
	int state; //0 if empty, 1 if paired
	int end;
	pthread_mutex_t mux;

} card_info;

typedef struct {
	int client_socket;
	int score;
} player_info;

typedef struct node{
	player_info client;
 	struct node *next;
}Node; 

int sock_fd;	// Socket to communicate
Node * Client_list;	// list that contains all the clients

void establish_client_connections (struct sockaddr_in *server_addr, struct sockaddr_in *addr);
void establish_server_connections ( struct sockaddr_in *address, int *server_fd);
int server_accept_client (struct sockaddr_in *address, int *server_fd);
void send_all_clients (card_info card);
Node * Add_Client (int new_client);
Node * Remove_Client (int client);