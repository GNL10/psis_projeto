#include <unistd.h>
#include <sys/un.h>
#include <pthread.h>
#include "UI_library.h"
#include "board_library.h"
#include "connections.h"

int done = 0;	// Variable used to know when the game ends

void *thread_update_board (void *arg) {
	play_response resp;

	while (!done) {
		recv(sock_fd, &resp, sizeof(resp), 0);
		paint_card(resp.play1[0], resp.play1[1] , 107, 200, 100);
        write_card(resp.play1[0], resp.play1[1], resp.str_play1, 0, 0, 0);
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
	
	while (!done){
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT: {
					done = SDL_TRUE;
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

	return 0;
}