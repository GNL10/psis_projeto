#include <stdlib.h>
#include "board_library.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>

int dim_board;
board_place * board;
int n_corrects;

int linear_conv(int i, int j){
  return j*dim_board+i;
}

void inverse_linear_conv (int i, int* x, int* y){
  *x = i%dim_board;
  *y = i/dim_board;
}

void print_board () {
  int x, y;

  printf("PRINTING BOARD \n\n");
  for (x = 0; x < dim_board; x++) {
    for (y = 0; y < dim_board; y++)
    {
      printf("%s ", board[linear_conv(y,x)].v);
    }
    printf("\n");
  }
}

// Returns adress of board[i][j]
char * get_board_place_str(int i, int j){
  return board[linear_conv(i, j)].v;
}


int unlock_board_mutex (int x, int y) {
  return pthread_mutex_unlock(&board[linear_conv(x,y)].mutex);
}


void init_board(int dim){
  int count  = 0;
  int i, j;
  char * str_place; // GNL -> Address of string in pos [i][j] of the board

  dim_board = dim;
  n_corrects = 0;
  board = (board_place*) malloc(sizeof(board_place)* dim *dim);

  if (board == NULL){
      printf("Allocation error\n");
      exit(EXIT_FAILURE);
  }
  
  srand(time(NULL));

  for( i=0; i < (dim_board*dim_board); i++){
    board[i].v[0] = '\0';
    board[i].card_color[0] = 255;
    board[i].card_color[1] = 255;
    board[i].card_color[2] = 255;
    pthread_mutex_init(&board[i].mutex, NULL);
  }

  for (char c1 = 'a' ; c1 < ('a'+dim_board); c1++){
  //for (char c1 = 'a' ; c1 < ('a'+(dim_board/2)); c1++){ // GNL -> Linha alterada por mim, para eliminar o if no fim do for
    for (char c2 = 'a' ; c2 < ('a'+dim_board) && c2 <= 'z'; c2++){
      // GNL -> Not very efficent ...
      do{
        i = rand()% dim_board;
        j = rand()% dim_board;
        str_place = get_board_place_str(i, j);
        //printf("%d %d -%s-\n", i, j, str_place);
      }while(str_place[0] != '\0');

      str_place[0] = c1;
      str_place[1] = c2;
      str_place[2] = '\0';
      do{
        i = rand()% dim_board;
        j = rand()% dim_board;
        str_place = get_board_place_str(i, j);
        //printf("%d %d -%s-\n", i, j, str_place);
      }while(str_place[0] != '\0');

      str_place[0] = c1;
      str_place[1] = c2;
      str_place[2] = '\0';
      count += 2;

      if (count == dim_board*dim_board) {
        print_board();
        return;
      }
      
    }
  }
}

play_response board_play(int x, int y, int play1[2]){
  play_response resp;
  //resp.code =10;

  printf("No board play: %d - %d -> %s\n", y, x, get_board_place_str(x, y));
  if(pthread_mutex_trylock(&board[linear_conv(x,y)].mutex) != 0){  // if card is locked
    if ((play1[0]==x) && (play1[1]==y)){
      resp.code = -1;
      resp.play1[0]= play1[0];
      resp.play1[1]= play1[1];
      play1[0] = -1;
      return resp;
    }
    printf("FILLED\n");
    resp.code = 0;
  }else{  
    // First play
    if(play1[0] == -1){
      printf("FIRST\n");
      resp.code =1;
      play1[0] = x;
      play1[1] = y;
      resp.play1[0]= x;
      resp.play1[1]= y;
      strcpy(resp.str_play1, get_board_place_str(x, y));
    // second play
    }else{
      char * first_str = get_board_place_str(play1[0], play1[1]);
      char * secnd_str = get_board_place_str(x, y);

      resp.play1[0]= play1[0];
      resp.play1[1]= play1[1];
      strcpy(resp.str_play1, first_str);
      resp.play2[0]= x;
      resp.play2[1]= y;
      strcpy(resp.str_play2, secnd_str);

      // if cards are a match
      if (strcmp(first_str, secnd_str) == 0){
        printf("CORRECT!!!\n");
        n_corrects +=2;

        // Game ends
        if (n_corrects == dim_board* dim_board)
            resp.code =3;

        // cards are a match, but it is not the end of the game
        else
          resp.code =2;
      }
      // if cards are not a match
      else{
        printf("INCORRECT\n");
        resp.code = -2;
      }
      play1[0]= -1;
    }
  }
  return resp;
}   

