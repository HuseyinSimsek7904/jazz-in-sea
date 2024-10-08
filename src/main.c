#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "board.h"
#include "position.h"
#include "move.h"

#define make_move(move)                           \
  printf("\nMaking move ");                       \
  print_move(move);                               \
  printf(".\n");                                  \
  do_move(&board, (move));                        \
  printf("New position:\n");                      \
  print_board(&board, false);


#define unmake_move(move)                         \
  printf("\nUnmaking move ");                     \
  print_move(move);                               \
  printf(".\n");                                  \
  undo_move(&board, (move));                      \
  printf("New position:\n");                      \
  print_board(&board, false);                     \


int
main() {
  board_t board;

  assert(load_fen("1n1P4/8/4P3/8/PP6/8/p6p/8", &board));
  printf("Successfully loaded FEN.\n");

  print_board(&board, false);

  move_t move1;
  move1.capture = INV_POSITION;
  move1.from = 0x24;
  move1.to = 0x34;
  move1.capture_piece = ' ';

  move_t move2;
  move2.from = 0x01;
  move2.capture = 0x03;
  move2.to = 0x04;
  move2.capture_piece = 'P';

  make_move(move1);
  make_move(move2);
  unmake_move(move2);
  unmake_move(move1);

  return 0;
}
