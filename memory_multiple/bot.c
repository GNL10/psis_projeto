#include <unistd.h>
#include <sys/un.h>
#include <pthread.h>
#include <stdio.h>
#include "board_library.h"
#include "connections.h"

int done = 0;	// Variable used to know when the game ends
board_place **board_known;	// board that stores all the cards that have been shown

void *thread_update_board (void *arg) {
	play_response resp;

	while (!done) {
		recv(sock_fd, &resp, sizeof(resp), 0);
		switch (resp.code) {
						case 1:	// First play -> write down the first card
							strcpy(board_known[resp.play1[0]][resp.play1[1]].v, resp.str_play1);
							break;
						case 3:	// End of the game
						  done = 1;
						case 2:	// Second play
							// Assign a default string to taken places
							strcpy(board_known[resp.play1[0]][resp.play1[1]].v, "\0");
							strcpy(board_known[resp.play2[0]][resp.play2[1]].v, "\0");
							break;
						case -2: // Wrong play -> write down the second card
							strcpy(board_known[resp.play2[0]][resp.play2[1]].v, resp.str_play2);
							break;
					}
	}
	return NULL;	// To ignore the warning
}

board_place ** init_bot_board (int dim) {
	int x, y;
	board_place **board;

	board = (board_place**)malloc(dim*sizeof(board_place*));
	for (x = 0; x < dim; x++) {
		printf("TESTE\n");
		board_known[x] = (board_place*) malloc(sizeof(board_place));
	}

	// reset the board
	for (x = 0; x < dim; x++)
		for (y = 0; y < dim; y++)
			strcpy (board_known[x][y].v, "HI");

	return board;
}

void print_bot_board (int dim) {
	int x, y;
	for (x = 0; x < dim; x++) {
		for (y = 0; y < dim; y++)
			printf("%s\n", board_known[x][y].v);
		printf("\n");
	}
}


int main(int argc, char const *argv[]) {
	struct sockaddr_in server_addr;
	struct sockaddr_in addr;
	pthread_t thread_id;
	int dim = 4; // In the future this will be received from the server

	//establish_client_connections(&server_addr, &addr);
	board_known = init_bot_board(dim);
	print_bot_board(dim);
	// Create thread that will receive and continuously update matrix
	/*pthread_create(&thread_id, NULL, thread_update_board, NULL);
	
	while (!done){
		int board_x= 0, board_y = 0;
		send(sock_fd, &board_x, sizeof(board_x), 0);
		send(sock_fd, &board_y, sizeof(board_y), 0);
	}*/

	return 0;
}