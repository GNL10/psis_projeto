#include <stdlib.h>
#include <time.h>

typedef struct board_place{
  char v[3];
  int card_state;	// 0 -> Down  1 -> Up  2 -> Locked
} board_place;

#define DOWN 0
#define UP 1
#define LOCKED 2

typedef struct play_response{
  int code; // 0 - filled
            // 1 - 1st play
            // 2 2nd - same plays
            // 3 END
            // -2 2nd - different
  int play1[2];
  int play2[2];
  char str_play1[3], str_play2[3];
} play_response;

int linear_conv(int i, int j);
char * get_board_place_str(int i, int j);
void set_card_state (int x, int y, int state);
int get_card_state (int x, int y);
void init_board(int dim);
play_response board_play (int x, int y, int play1[2]);
