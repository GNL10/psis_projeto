#include <unistd.h>
#include <sys/un.h>
#include <pthread.h>
#include <stdio.h>
#include "board_library.h"
#include "connections.h"

int done = 0;	// Variable used to know when the game ends
board_place *board_known;	// board that stores all the cards that have been shown
int dim;


int lin_conv (int x, int y);
board_place * init_bot_board ();
void print_bot_board ();
void *thread_update_board (void *arg);
int assign_code (card_info card);
int find_pair (int *x1, int *y1, int *x2, int *y2);
void random_play (int *x, int *y);


int main(int argc, char const *argv[]) {
	struct sockaddr_in server_addr;
	struct sockaddr_in addr;
	//pthread_t thread_id;
	int x1, y1;
	int x2, y2;

	srand(getpid());
	establish_client_connections(&server_addr, &addr);
	
	read(sock_fd, &dim, sizeof(dim));
	board_known = init_bot_board();
	// Create thread that will receive and continuously update matrix
	//pthread_create(&thread_id, NULL, thread_update_board, NULL);

	while (1){
		random_play(&x1, &y1);
		send(sock_fd, &x1, sizeof(x1), 0);
		send(sock_fd, &y1, sizeof(y1), 0);
		sleep(2);
		random_play(&x2, &y2);
		send(sock_fd, &x2, sizeof(x2), 0);
		send(sock_fd, &y2, sizeof(y2), 0);
		sleep(6);

		/*if (find_pair(&x1, &y1, &x2, &y2) == 1) {
			printf("AVAILABLE PAIR in (%d - %d) and (%d - %d)!!!\n", y1, x1, y2, x2 );
			send(sock_fd, &x1, sizeof(x1), 0);
			send(sock_fd, &y1, sizeof(y1), 0);
			send(sock_fd, &x2, sizeof(x2), 0);
			send(sock_fd, &y2, sizeof(y2), 0);
		}
		else {
			random_play(&x1, &y1);
			printf("Bot played : %d - %d -> %s\n", y1, x1, board_known[lin_conv(x1,y1)].v);
			send(sock_fd, &x1, sizeof(x1), 0);
			send(sock_fd, &y1, sizeof(y1), 0);

			random_play(&x2, &y1);
			printf("Bot played : %d - %d -> %s\n", y2, x2, board_known[lin_conv(x2,y2)].v);
			send(sock_fd, &x2, sizeof(x2), 0);
			send(sock_fd, &y2, sizeof(y2), 0);
		}
		sleep(6);*/
	}

	return 0;
}


int lin_conv (int x, int y) {
	return y*dim+x;
}

/*	init_bot_board
	Allocates memory for the bot's auxiliar board and initializes every position to "\0"
*/
board_place * init_bot_board () {
	int i;
	board_place *board;

	board = (board_place*) malloc(sizeof(board_place)* dim *dim);

	// reset the board
	for( i=0; i < (dim*dim); i++){
		strcpy(board[i].v, "\0");
  	}
	return board;
}

/*	function print_boat_board
	prints the board with the information that the bot has stored at the moment
*/
void print_bot_board () {
	int x, y;

	printf("Printing updated board\n\n");
	for (x = 0; x < dim; x++) {
		for (y = 0; y < dim; y++)
			printf("%s ", board_known[lin_conv(y, x)].v);
		printf("\n");
	}
}

/*	thread_update_board	
	This function is implemented in a thread, and throughout the game it receives and updates the
	bot's auxiliar board
*/
void *thread_update_board (void *arg) {
	card_info card;
	int code;
	int read_size;
	char* str = malloc(sizeof(card_info));
	
	// use the colors of the letters to know which situation we are in
	// card color = 255 255 255 -> card available
	// black letters 0 0 0 -> pair not available
	// 200 200 200 -> first play -> save the card
		
	while (!done) {
		read_size = read(sock_fd, str, sizeof(card_info));
		if (read_size == 0)
			break;
		memcpy(&card, str, sizeof(card_info));
		printf("Bot received: %d - %d with string %s\n", card.y, card.x, card.string);
		
		if (card.end == 1) {	// game finished -> clear board and get ready for another game
			printf("bot restarting board\n");
			free(board_known);
			init_bot_board();
		}
		else {
			code = assign_code(card);
			switch (code) {
				case 1:	// First play -> write down the first card
					strcpy(board_known[lin_conv(card.x, card.y)].v, card.string);
					break;
				case 3:	// End of the game
				 	done = 1;
				case 2:	// Second play
					// Assign a default string to taken places
					printf("Found pair\n");
					strcpy(board_known[lin_conv(card.x, card.y)].v, "--");
					break;
				case -2: // Wrong play -> write down the second card
					strcpy(board_known[lin_conv(card.x, card.y)].v, card.string);
					break;
			}
		}
		print_bot_board();
	}
	return NULL;	// To ignore the warning
}

int assign_code (card_info card) {
	// end of the game
	if (card.end == 1)
		return 3;
	// first play
	else if (card.string_color[0] == 200 && card.string_color[1] == 200 && card.string_color[2] == 200)
		return 1;
	// pair found, card no longer available
	else if (card.string_color[0] == 0 && card.string_color[1] == 0 && card.string_color[2] == 0)
		return 2;
	// wrong play -> pair was not a match
	else if (card.string_color[0] == 255 && card.string_color[1] == 0 && card.string_color[2] == 0)
		return -2;
	return -1;
}

int find_pair (int *x1, int *y1, int *x2, int *y2) {
	int i1, i2;

	// dim*dim-1 because if i1 is the last element, then it has no pair
	for (i1 = 0; i1 < dim*dim-1; i1++) {
		// if the position is neither a \0 nor a -- then it is a string	
		if ((strcmp(board_known[i1].v, "\0") != 0) && (strcmp(board_known[i1].v, "--") != 0)) {		
			for (i2 = i1+1; i2 < dim*dim; i2++) {
				if (strcmp(board_known[i1].v, board_known[i2].v) == 0){
					*x1 = i1 % dim;
					*y1 = i1 / dim;
					*x2 = i2 % dim;
					*y2 = i2 / dim;
					return 1;
				}
			}
		}
	}
	return 0;
}

/*	random_play
	makes a random play, avoiding the cards that have been matched
*/
void random_play (int *x, int *y) {
		do {
			*x = rand()%dim;
		 	*y = rand()%dim;
		 	// IMPORTANT: while playing in multiplayer, if there are no cards left this will do an infinite loop
		} while (strcmp(board_known[lin_conv(*x,*y)].v, "\0") != 0);
}