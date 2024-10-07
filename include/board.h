#include <stdbool.h>

typedef struct {
  char board_array[8][8];
} board_t;


char get_piece(board_t*, int, int);
void set_piece(board_t*, int, int, char);

bool load_fen(const char*, board_t*);
void print_board(board_t*, bool);
