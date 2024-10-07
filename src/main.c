#include <stdio.h>
#include <stdlib.h>

#include "board.h"

int
main() {
  printf("Hello, world!\n");
  board_t board;
  if (load_fen("np4PN/pp4PP/8/8/8/8/PP4pp/NP4pn", &board)) {
    printf("Successfully loaded FEN.\n");
  } else {
    printf("Could not load FEN.\n");
    exit(1);
  }

  printf("\nBoard from white's perspective: \n");
  print_board(&board, false);

  printf("\nBoard from black's perspective: \n");
  print_board(&board, true);
  return 0;
}
