#include <sys/socket.h>
#include <arpa/inet.h>
// colocar estas bibliotecas no .c

#define PORT 3000

typedef struct {
	int x;
	int y;
	int card_color[3];	// R G B
	char string[3];
	int string_color[3];
	int end; // 0 if game is still running, 1 if game has finished
	int state; //0 if empty, 1 if paired
} card_info;

typedef struct node{
 	int client_socket;
 	int score;
 	struct node *next;
} Node; 

int sock_fd;	// Socket to communicate

void establish_client_connections (struct sockaddr_in *server_addr, struct sockaddr_in *addr);
void establish_server_connections ( struct sockaddr_in *address, int *server_fd);
int server_accept_client (struct sockaddr_in *address, int *server_fd);