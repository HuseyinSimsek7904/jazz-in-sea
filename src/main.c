#include <stdio.h>
#include <stdlib.h>

#include "board.h"

int
main() {
  board_t board;
  if (!load_fen("1n6/8/4P3/8/PP6/8/p6p/8", &board)) {
    printf("Could not load FEN.\n");
    exit(1);
  }

  printf("\nBoard from white's perspective: \n");
  print_board(&board, false);

  printf("\nBoard from black's perspective: \n");
  print_board(&board, true);
  return 0;
}
