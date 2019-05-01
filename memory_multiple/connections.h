#include <sys/socket.h>
#include <arpa/inet.h>
// colocar estas bibliotecas no .c

#define PORT 3000

int sock_fd;	// Socket to communicate

void establish_client_connections (struct sockaddr_in *server_addr, struct sockaddr_in *addr);
void establish_server_connections ( struct sockaddr_in *address, int *server_fd);
int server_accept_client (struct sockaddr_in *address, int *server_fd);