#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "board.h"
#include "position.h"
#include "move.h"
#include "rules.h"


void search_depth(board_t* board, int depth) {
  if (depth == 0) return;

  move_t moves[256];
  printf("Trying to generate moves...\n");
  int length = generate_moves(board, moves);

  if (!length) {
    print_board(board, false);
    printf("No moves available, going back.\n");
    return;
  }

  for (int i=0; i<length - 1; i++) {
    print_move(moves[i]);
    printf(", ");
  }

  printf("%u available moves: ", length);
  print_move(moves[length - 1]);
  printf("\n");

  move_t move = moves[rand() % length];

  printf("Making move ");
  print_move(move);
  printf(".\n");

  do_move(board, move);

  search_depth(board, depth - 1);

  printf("Unmaking move ");
  print_move(move);
  printf(".\n");

  undo_move(board, move);
}


int
main() {
  board_t board;
  board.turn = true;

  srand(time(NULL));

  assert(load_fen(DEFAULT_BOARD, &board));
  printf("Successfully loaded FEN.\n");

  print_board(&board, false);

  search_depth(&board, -1);

  return 0;
}
