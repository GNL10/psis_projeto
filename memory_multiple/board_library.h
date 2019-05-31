#ifndef _BOARD_LIBRARY_H_
#define _BOARD_LIBRARY_H_ 

#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

typedef struct board_place{
  char v[3];
  int card_color[3];
  int string_color[3];
  pthread_mutex_t mutex;
} board_place;

typedef struct play_response{
  int code; // 0 - filled
            // 1 - 1st play
            // 2 2nd - same plays
            // 3 END
            // -2 2nd - different
  			// -1 picked the same card
  int play1[2];
  int play2[2];
  char str_play1[3], str_play2[3];
} play_response;

int linear_conv(int i, int j);
void inverse_linear_conv (int i, int* x, int* y);
void print_board ();
char * get_board_place_str(int i, int j);
int unlock_board_mutex (int x, int y);
void init_board();
play_response board_play (int x, int y, int play1[2]);
void Update_Board (board_place *board, int c_color[3], int s_color[3]);

#endif