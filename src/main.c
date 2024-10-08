#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "board.h"
#include "position.h"

int
main() {
  board_t board;

  assert(load_fen("1n6/8/4P3/8/PP6/8/p6p/8", &board));
  printf("Successfully loaded FEN.\n");

  printf("\nBoard from white's perspective: \n");
  print_board(&board, false);

  printf("\nBoard from black's perspective: \n");
  print_board(&board, true);

  printf("\nMaking move from ");
  print_position(0x24);
  printf(" to ");
  print_position(0x34);
  printf(".\n ");

  return 0;
}
