#include "UI_library.h"

int screen_width;
int screen_height;
int n_ronw_cols;
int row_height;
int col_width;
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

/* 	function write_card
	renders the string text with the color {r,g,b} on top of the position board_x board_y
*/
void write_card(int  board_x, int board_y, char * text, int r, int g, int b){
	SDL_Rect rect;

	rect.x = board_x * col_width;
	rect.y = board_y * row_height;
	rect.w = col_width+1;
	rect.h = row_height+1;

	// loads font
	TTF_Font * font = TTF_OpenFont("arial.ttf", row_height);

	SDL_Color color = { r, g, b };
 	SDL_Surface * surface = TTF_RenderText_Solid(font, text, color);

	SDL_Texture* Background_Tx = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface); /* we got the texture now -> free surface */
	TTF_CloseFont(font);

	SDL_RenderCopy(renderer, Background_Tx, NULL, &rect);
	SDL_RenderPresent(renderer);
	SDL_Delay(DELAY_TIME);
}

/* 	function paint_card
	paints a square with the color {r,g,b} on the position board_x board_y
*/
void paint_card(int  board_x, int board_y , int r, int g, int b){
	SDL_Rect rect;

	rect.x = board_x * col_width;
	rect.y = board_y * row_height;
	rect.w = col_width+1;
	rect.h = row_height+1;

	// draw black lines on the border of the card
	SDL_SetRenderDrawColor(renderer, r, g, b, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, &rect);

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderDrawRect(renderer, &rect);

	SDL_RenderPresent(renderer);
	SDL_Delay(DELAY_TIME);
}

/* 	function clear_card
	paints a white square on the position board_x board_y
*/
void clear_card(int  board_x, int board_y){
	paint_card(board_x, board_y , 255, 255, 255);
}

/* 	function get_board_card
	reads mouse coordinates and returns matrix alike coordinates
*/
void get_board_card(int mouse_x, int mouse_y, int *board_x, int *board_y){
	*board_x = mouse_x / col_width;
	*board_y = mouse_y / row_height;
}

/* 	function create_board_window
	Creates and draws the graphical window with white cards
*/
int create_board_window(int width, int height,  int dim){

	screen_width = width;
	screen_height = height;
	n_ronw_cols = dim;
	row_height = height /n_ronw_cols;
	col_width = width /n_ronw_cols;
	screen_width = n_ronw_cols * col_width +1;
	screen_height = n_ronw_cols *row_height +1;
	int i;

	if (SDL_CreateWindowAndRenderer(screen_width, screen_height, 0, &window, &renderer)  != 0) {
		printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
		exit(-1);
	}

	// Paints the window with white
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);

	
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	// draws the horizontal lines
	for (i = 0; i <n_ronw_cols+1; i++){
		SDL_RenderDrawLine(renderer, 0, i*row_height, screen_width, i*row_height);
	}
	// draws the vertical lines
	for (i = 0; i <n_ronw_cols+1; i++){
		SDL_RenderDrawLine(renderer, i*col_width, 0, i*col_width, screen_height);
	}
	SDL_RenderPresent(renderer);
	SDL_Delay(DELAY_TIME);
	return 0;
}


/* 	function close_board_windows
	closes the graphical window
*/
void close_board_windows(){
	if (renderer) {
		SDL_DestroyRenderer(renderer);
	}
	if (window) {
		SDL_DestroyWindow(window);
	}
}
