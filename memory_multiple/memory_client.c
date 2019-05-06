#include <unistd.h>
#include <sys/un.h>
#include <pthread.h>
#include "UI_library.h"
#include "board_library.h"
#include "connections.h"

int DONE = 0;	// Variable used to know when the game ends

void *thread_update_board (void *arg) {
	card_info card;

	while (!DONE) {
		recv(sock_fd, &card, sizeof(card), 0);
		printf("Card %d - %d, RGB: %d %d %d\n",card.x, card.y, card.card_color[0], card.card_color[1], card.card_color[2] );
		paint_card(card.x, card.y, card.card_color[0], card.card_color[1], card.card_color[2]);
		// If the card isn't white, print the string
		if (! (card.card_color[0] == 255 && card.card_color[1] == 255 && card.card_color[2] == 255))	
        	write_card(card.x, card.y, card.string, card.string_color[0], card.string_color[1], card.string_color[2]);
		DONE = card.end;
	}	
	return NULL;	// To ignore the warning
}


int main(int argc, char const *argv[]) {
	struct sockaddr_in server_addr;
	struct sockaddr_in addr;
	SDL_Event event;
	pthread_t thread_id;

	establish_client_connections(&server_addr, &addr);

	if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
		 printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
		 exit(-1);
	}
	if(TTF_Init()==-1) {
			printf("TTF_Init: %s\n", TTF_GetError());
			exit(2);
	}

	create_board_window(300, 300,  4);
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

					printf("click (%d %d) -> (%d %d)\n", event.button.x, event.button.y, board_x, board_y);
					// send play to server
					send(sock_fd, &board_x, sizeof(board_x), 0);
					send(sock_fd, &board_y, sizeof(board_y), 0);
				}
			}
		}
	}
	close_board_windows();	// not sure
	TTF_Quit();
	SDL_Quit();
	close(sock_fd);
	return 0;
}