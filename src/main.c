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

void depth_test(int argc, const char** argv) {
  int depth = -1;
  if (argc > 1) {
    depth = atoi(argv[1]);
  }

  printf("Trying to print a branch from root to a leaf with depth %i...\n", depth);

  board_t board;
  board.turn = true;
  board_t board_original;

  assert(load_fen(DEFAULT_BOARD, &board));
  copy_board(&board, &board_original);

  printf("Successfully loaded FEN.\n");

  print_board(&board, false);

  search_depth(&board, depth);

  printf("Resulting board:\n");
  print_board(&board, false);

  if (compare(&board, &board_original)) {
    printf("Boards match!\n");
    exit(0);
  } else {
    printf("Boards do not match!\n");
    exit(1);
  }
}

int main(int argc, const char** argv) {
  srand(time(NULL));

  int option = 0;
  if (argc > 0) {
    option = atoi(argv[0]);
  }

  switch (option) {
  case 0: depth_test(argc, argv); break;
  default:
    break;
      }
}
