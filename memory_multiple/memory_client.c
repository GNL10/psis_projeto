#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/un.h>
#include <pthread.h>
#include "UI_library.h"
#include "board_library.h"

#define PORT 8080

int sock_fd;	// Socket to communicate
int done = 0;	// Variable used to know when the game ends

void establish_connections (struct sockaddr_in *server_addr, struct sockaddr_in *addr) {
	int error;

	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd == -1){
		perror("socket: ");
		exit(-1);
	}

	memset(server_addr, '0', sizeof(*server_addr));
	addr->sin_family = AF_INET;
	server_addr->sin_port = htons(PORT);

	if(inet_pton(AF_INET, "127.0.0.1", &server_addr->sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        exit (-1); 
    } 

	server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(PORT); 
 	error = connect(sock_fd, (struct sockaddr *)server_addr, sizeof(*server_addr));
	if(error == -1) {
		perror("connect");
		exit(-1);
	}
}

void *thread_update_board (void *arg) {
	play_response resp;

	while (!done) {
		recv(sock_fd, &resp, sizeof(resp), 0);
		switch (resp.code) {
						case 1:
							paint_card(resp.play1[0], resp.play1[1] , 7, 200, 100);
							write_card(resp.play1[0], resp.play1[1], resp.str_play1, 200, 200, 200);
							break;
						case 3:
						  done = 1;
						case 2:
							paint_card(resp.play1[0], resp.play1[1] , 107, 200, 100);
							write_card(resp.play1[0], resp.play1[1], resp.str_play1, 0, 0, 0);
							paint_card(resp.play2[0], resp.play2[1] , 107, 200, 100);
							write_card(resp.play2[0], resp.play2[1], resp.str_play2, 0, 0, 0);
							break;
						case -2:
							paint_card(resp.play1[0], resp.play1[1] , 107, 200, 100);
							write_card(resp.play1[0], resp.play1[1], resp.str_play1, 255, 0, 0);
							paint_card(resp.play2[0], resp.play2[1] , 107, 200, 100);
							write_card(resp.play2[0], resp.play2[1], resp.str_play2, 255, 0, 0);
							sleep(2);
							paint_card(resp.play1[0], resp.play1[1] , 255, 255, 255);
							paint_card(resp.play2[0], resp.play2[1] , 255, 255, 255);
							break;
					}
	}
}


int main(int argc, char const *argv[]) {
	struct sockaddr_in server_addr;
	struct sockaddr_in addr;
	SDL_Event event;
	pthread_t thread_id;

	establish_connections(&server_addr, &addr);

	if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
		 printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
		 exit(-1);
	}
	if(TTF_Init()==-1) {
			printf("TTF_Init: %s\n", TTF_GetError());
			exit(2);
	}

	//test
	// Vai dar errog
	//test2


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