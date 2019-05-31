#include "board_library.h"
#include "connections.h"

int DIM;
extern int SOCK_FD;	// from connections.c

void random_play (int *x, int *y);

int main(int argc, char const *argv[]) {
	struct sockaddr_in server_addr;
	struct sockaddr_in addr;
	int x1, y1;
	int x2, y2;

	srand(getpid());
	establish_client_connections(&server_addr, &addr);
	
	read(SOCK_FD, &DIM, sizeof(DIM));

	while (1){
		random_play(&x1, &y1);
		send(SOCK_FD, &x1, sizeof(x1), 0);
		send(SOCK_FD, &y1, sizeof(y1), 0);
		sleep(2);
		random_play(&x2, &y2);
		send(SOCK_FD, &x2, sizeof(x2), 0);
		send(SOCK_FD, &y2, sizeof(y2), 0);
		sleep(4);
	}

	return 0;
}

/*	function random_play
	makes a random play withing the DIM limits
*/
void random_play (int *x, int *y) {
	*x = rand()%DIM;
	*y = rand()%DIM;
}
