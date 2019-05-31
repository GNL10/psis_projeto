#include "board_library.h"

int BOARD_SIZE = 0;   // Size of the board
board_place * BOARD = NULL;  // Will hold the list with the board and the information about every position
int N_CORRECTS;       // Number of total correct plays

/*  function linear_conv
    transforms x,y format into the board format
*/
int linear_conv(int i, int j){
  return j*BOARD_SIZE+i;
}

/*  function inverse_linear_conv
    transforms board format into x,y format
*/
void inverse_linear_conv (int i, int* x, int* y){
  *x = i%BOARD_SIZE;
  *y = i/BOARD_SIZE;
}

/*  function get_board_place_str
    returns the address of the string in the i,j position of the BOARD 
*/
char * get_board_place_str(int i, int j){
  return BOARD[linear_conv(i, j)].v;
}

/*  function unlock_board_mutex
    unlocks the mutex of the i, j position in the BOARD
*/
int unlock_board_mutex (int x, int y) {
  return pthread_mutex_unlock(&BOARD[linear_conv(x,y)].mutex);
}

/*  function print_board
    prints the whole board
*/
void print_board () {
  int x, y;

  if (BOARD == NULL){
    printf("print_board: BOARD has NULL address!\n");
    exit(EXIT_FAILURE);
  }

  printf("PRINTING BOARD \n\n");
  for (x = 0; x < BOARD_SIZE; x++) {
    for (y = 0; y < BOARD_SIZE; y++)
    {
      printf("%s ", BOARD[linear_conv(y,x)].v);
    }
    printf("\n");
  }
}

/*  function init_board
    allocates the memory for the BOARD
    initializes all the BOARD positions
    creates the BOARD to be played on, through random operations
*/
void init_board(){
  int count  = 0;
  int i, j;
  char * str_place;
  char c1, c2;

  N_CORRECTS = 0; // reset in case of new game
  BOARD = (board_place*) malloc(sizeof(board_place)* BOARD_SIZE*BOARD_SIZE);
  if (BOARD == NULL){
      printf("Allocation error\n");
      exit(EXIT_FAILURE);
  }

  srand(time(NULL));

  // initializes the colors and strings for every position
  for( i=0; i < (BOARD_SIZE*BOARD_SIZE); i++){
    BOARD[i].v[0] = '\0';
    BOARD[i].card_color[0] = 255;
    BOARD[i].card_color[1] = 255;
    BOARD[i].card_color[2] = 255;
    pthread_mutex_init(&BOARD[i].mutex, NULL);
  }

  // place the strings randomly on the BOARD
  for (c1 = 'a' ; c1 < ('a'+BOARD_SIZE); c1++){
    for (c2 = 'a' ; c2 < ('a'+BOARD_SIZE) && c2 <= 'z'; c2++){
      do{
        i = rand()% BOARD_SIZE;
        j = rand()% BOARD_SIZE;
        str_place = get_board_place_str(i, j);
      }while(str_place[0] != '\0');

      str_place[0] = c1;
      str_place[1] = c2;
      str_place[2] = '\0';
      do{
        i = rand()% BOARD_SIZE;
        j = rand()% BOARD_SIZE;
        str_place = get_board_place_str(i, j);
      }while(str_place[0] != '\0');

      str_place[0] = c1;
      str_place[1] = c2;
      str_place[2] = '\0';
      count += 2;

      if (count == BOARD_SIZE*BOARD_SIZE) { // when all the positions are filled
        print_board();
        return;
      } 
    }
  }
}

/*  function board_play
    processes the play according to the current BOARD
    returns: -1 if the first card is picked again, after being picked on the first time
              0 if the card is filled (locked)
              1 if it is the first play
             -2 if the cards have different strings
              2 if the cards have the same string, but the game is not over
              3 if the game is over
*/
play_response board_play(int x, int y, int play1[2]){
  play_response resp;

  printf("No board play: %d - %d -> %s\n", y, x, get_board_place_str(x, y));
  // if card is locked
  if(pthread_mutex_trylock(&BOARD[linear_conv(x,y)].mutex) != 0){
    if ((play1[0]==x) && (play1[1]==y)){  // picked the first card again
      resp.code = -1;
      resp.play1[0]= play1[0];
      resp.play1[1]= play1[1];
      play1[0] = -1;
      return resp;
    }
    printf("FILLED\n");
    resp.code = 0;  // card locked and is not the first pick -> means it is filled
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
        N_CORRECTS +=2;

        // Game ends
        if (N_CORRECTS == BOARD_SIZE* BOARD_SIZE)
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
      play1[0]= -1; // after the second play, reset the first one
    }
  }
  return resp;
}   

/*  function Update_Board
    updates the position of the board with the input colors
*/
void Update_Board (board_place *board, int c_color[3], int s_color[3]) {
    board->card_color[0] = c_color[0];
    board->card_color[1] = c_color[1];
    board->card_color[2] = c_color[2];
    board->string_color[0] = s_color[0];
    board->string_color[1] = s_color[1];
    board->string_color[2] = s_color[2];
}   