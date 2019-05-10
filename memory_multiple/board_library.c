#include <stdlib.h>
#include "board_library.h"
#include <stdio.h>
#include <string.h>

int dim_board;
board_place * board;
int n_corrects;

int linear_conv(int i, int j){
  return j*dim_board+i;
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

void set_card_state (int x, int y, int state) {
  board[linear_conv(x,y)].card_state = state;
}

int get_card_state (int x, int y) {
  return board[linear_conv(x,y)].card_state;
}

void init_board(int dim){
  //int count  = 0;
  int i, j;
  char * str_place; // GNL -> Address of string in pos [i][j] of the board

  dim_board = dim;
  n_corrects = 0;
  board = (board_place*) malloc(sizeof(board_place)* dim *dim);
  srand(time(NULL));

  for( i=0; i < (dim_board*dim_board); i++){
    board[i].v[0] = '\0';
    board[i].card_state = 0;
  }

  // Linha original do prof -> for (char c1 = 'a' ; c1 < ('a'+dim_board); c1++){
  for (char c1 = 'a' ; c1 < ('a'+(dim_board/2)); c1++){ // GNL -> Linha alterada por mim, para eliminar o if no fim do for
    for (char c2 = 'a' ; c2 < ('a'+dim_board); c2++){
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
      /*count += 2;
      if (count == dim_board*dim_board)
        return;
      */
    }
  }
  print_board();
}

play_response board_play(int x, int y, int play1[2]){
  play_response resp;
  resp.code =10;

  printf("No board play: %d - %d -> %s\n", x, y, get_board_place_str(x, y));
  if(board[linear_conv(x,y)].card_state == LOCKED || board[linear_conv(x,y)].card_state == UP){
    printf("FILLED\n");
    resp.code =0;
  }else{
    // if card = DOWN, 5 seconds have passed and the timeout has occurred
    if(play1[0] == -1 || get_card_state(play1[0], play1[1]) == DOWN){
        printf("FIRST\n");
        resp.code =1;
        play1[0] = x;
        play1[1] = y;
        resp.play1[0]= x;
        resp.play1[1]= y;
        strcpy(resp.str_play1, get_board_place_str(x, y));
        
        board[linear_conv(x,y)].card_state = UP;  // Card is now UP
      }else{
        char * first_str = get_board_place_str(play1[0], play1[1]);
        char * secnd_str = get_board_place_str(x, y);

        if ((play1[0]==x) && (play1[1]==y)){
          resp.code = 0;
          printf("FILLED\n");
        } else{
          resp.play1[0]= play1[0];
          resp.play1[1]= play1[1];
          strcpy(resp.str_play1, first_str);
          resp.play2[0]= x;
          resp.play2[1]= y;
          strcpy(resp.str_play2, secnd_str);

          if (strcmp(first_str, secnd_str) == 0){
            printf("CORRECT!!!\n");

            // Lock both the cards!
            board[linear_conv(resp.play1[0],resp.play1[1])].card_state = LOCKED;
            board[linear_conv(resp.play2[0],resp.play2[1])].card_state = LOCKED;

            n_corrects +=2;

            // Game ends
            if (n_corrects == dim_board* dim_board)
                resp.code =3;

            // Same plays
            else
              resp.code =2;
          }else{
            printf("INCORRECT\n");
            // Cards are LOCKED for 2 Seconds and then are set to DOWN in the thread that sends the cards to the clients
            board[linear_conv(resp.play1[0],resp.play1[1])].card_state = LOCKED;
            board[linear_conv(resp.play2[0],resp.play2[1])].card_state = LOCKED;
            resp.code = -2;
          }
          play1[0]= -1;
        }
      }
  }
  return resp;
}   