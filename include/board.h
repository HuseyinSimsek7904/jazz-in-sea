#pragma once

#include <stdbool.h>

#define DEFAULT_BOARD "np4PN/pp4PP/8/8/8/8/PP4pp/NP4pn"

typedef struct {
  #ifndef NDEBUG
  bool initialized;
  #endif

  // A board_array is a 16x16 array which contains information about each cell on a board.
  // The index of this array is called the POSITION.
  // A position can be a number between 0-255, however there are only 64 cells on a board.
  // Only the 0th-2nd and 4th-6th bits on a position is actually used, these is where the main board is located.
  // The remaining space is empty.
  // This extra space in positions are used for checking whether a position is valid.
  // So checking if (POS & 0x88) == 0 is enough to check if the position is out of the bounds of the board.
  // This will hopefully allow for fast checking of bounds.
  char board_array[16 * 16];

  // false -> black, true -> white
  bool turn;
} board_t;


char get_piece(board_t*, int);
void set_piece(board_t*, int, char);

bool load_fen(const char*, board_t*);
void print_board(board_t*, bool);

bool next_turn(board_t*);

inline bool whites_turn(board_t *board) { return  board->turn; }
inline bool blacks_turn(board_t *board) { return !board->turn; }

void copy_board(board_t*, board_t*);

bool compare(board_t*, board_t*);
