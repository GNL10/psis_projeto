#include "UI_library.h"
#include "board_library.h"
#include "connections.h"

int DONE = 0;	// Variable used to know when the client exits
extern int SOCK_FD;	// from connections.c

void *thread_update_board (void *arg);

int main(int argc, char const *argv[]) {
	struct sockaddr_in server_addr;
	struct sockaddr_in addr;
	SDL_Event event;
	pthread_t thread_id;
	int board_dim;

	establish_client_connections(&server_addr, &addr);

	if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
		 printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
		 exit(-1);
	}
	if(TTF_Init()==-1) {
			printf("TTF_Init: %s\n", TTF_GetError());
			exit(2);
	}

	if (read(SOCK_FD, &board_dim, sizeof(board_dim)) == -1)
		DONE = 1;
	create_board_window(400, 400, board_dim);

	// Create thread that will receive and continuously update the graphical interface
	pthread_create(&thread_id, NULL, thread_update_board, NULL);
	
	while (!DONE){
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT: {
					DONE = SDL_TRUE;
					break;
				}
				case SDL_MOUSEBUTTONDOWN:{
					int board_x, board_y;
					get_board_card(event.button.x, event.button.y, &board_x, &board_y);

					//printf("click (%d %d) -> (%d %d)\n", event.button.x, event.button.y, board_x, board_y);
					// send play to server

					// if any of the sends returns -1 it means that the server was closed, it will then exit
					if (send(SOCK_FD, &board_x, sizeof(board_x), 0) == -1)
						DONE = 1;
					if (send(SOCK_FD, &board_y, sizeof(board_y), 0) == -1)
						DONE = 1;
				}
			}
		}
	}
	close_board_windows();
	TTF_Quit();
	SDL_Quit();
	close(SOCK_FD);
	return 0;
}

/* 	function thread_update_board
	thread that constantly updates the graphical board of the client
*/
void *thread_update_board (void *arg) {
	card_info card;
	char* str = malloc(sizeof(card_info));
	if (str == NULL){
      printf("Allocation error\n");
      exit(EXIT_FAILURE);
  	}

	while (1) {
		// if the size returned by the read is 0, connection has been lost
		if (read(SOCK_FD, str, sizeof(card_info)) == 0) {
			DONE = 1;
			printf("Client lost connection\n");
			break;
		}
		memcpy(&card, str, sizeof(card_info));
		// Informs player that it won the game
		if(card.winner_score != 0){
			printf("You have won the game with %d points\n", card.winner_score);
			continue;
		}
		//printf("Card %d - %d, RGB: %d %d %d\n",card.x, card.y, card.card_color[0], card.card_color[1], card.card_color[2] );
		paint_card(card.x, card.y, card.card_color[0], card.card_color[1], card.card_color[2]);
		// If the card isn't white, print the string
		if (! (card.card_color[0] == 255 && card.card_color[1] == 255 && card.card_color[2] == 255))	
        	write_card(card.x, card.y, card.string, card.string_color[0], card.string_color[1], card.string_color[2]);
	}	
	free(str);
	pthread_exit(0);
}