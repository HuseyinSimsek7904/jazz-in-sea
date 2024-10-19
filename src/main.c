#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "ai.h"
#include "board.h"
#include "position.h"
#include "move.h"
#include "rules.h"
#include "fen.h"
#include "io.h"


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

int count_branches(board_t* board, int depth) {
  if (depth == 0) return 1;

  move_t moves[256];
  int length = generate_moves(board, moves);

  int total = 0;
  for (int i=0; i<length; i++) {
    do_move(board, moves[i]);
    total += count_branches(board, depth - 1);
    undo_move(board, moves[i]);
  }
  return total;
}

void depth_test(int argc, const char** argv) {
  int depth = -1;
  if (argc >= 3) {
    depth = atoi(argv[2]);
  }

  printf("Trying to print a branch from root to a leaf with depth %i...\n", depth);

  board_t board;
  board_t board_original;

  if (!load_fen(DEFAULT_BOARD, &board)) {
    printf("Could not load FEN.\n");
    exit(1);
  }
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

void count_test(int argc, const char** argv) {
  if (argc < 3) {
    printf("Count test requires at least one argument.\n");
    exit(1);
  }

  int depth = atoi(argv[2]);

  const char* fen = DEFAULT_BOARD;
  if (argc >= 4) {
    fen = argv[3];
  }

  board_t board;

  if (!load_fen(fen, &board)) {
    printf("Could not load FEN.\n");
    exit(1);
  }

  int branches = count_branches(&board, depth);
  printf("%u\n", branches);

  exit(0);
}

void state_test(int argc, const char** argv) {
  const char* fen = DEFAULT_BOARD;
  if (argc >= 3) {
    fen = argv[2];
  }

  board_t board;

  if (!load_fen(fen, &board)) {
    printf("Could not load FEN.\n");
    exit(1);
  }

  state_t state = get_board_state(&board);
  printf("%s\n", board_state_text(state));

  exit(0);
}

void distance_test(int argc, const char** argv) {
  printf("Pawn distance:\n");
  for (int prow=0; prow<8; prow++) {
    for (int pcol=0; pcol<8; pcol++) {
      pos_t pos = to_position(perspective_row(prow, false),
                              perspective_col(pcol, false));

      printf("%c", '0' + pawn_dist_to_center(pos));
    }
    printf("\n");
  }

  printf("Knight distance:\n");
  for (int prow=0; prow<8; prow++) {
    for (int pcol=0; pcol<8; pcol++) {
      pos_t pos = to_position(perspective_row(prow, false),
                              perspective_col(pcol, false));

      printf("%c", '0' + knight_dist_to_center(pos));
    }
    printf("\n");
  }
}

void iterative_ai_test(int argc, const char** argv) {
  const char* fen = DEFAULT_BOARD;
  size_t depth = 3;

  if (argc >= 3) {
    depth = atoi(argv[2]);
    if (argc >= 4) {
      fen = argv[3];
    }
  }

  board_t board;
  if (!load_fen(fen, &board)) {
    printf("Could not load fen.\n");
    exit(1);
  }

  while (true) {
    print_board(&board, false);

    eval_t evaluation;
    move_t move = evaluate(&board, depth, &evaluation);

    print_eval(evaluation, &board);

    #ifdef EVALCOUNT
    printf("total %u calls to eval\n", get_evaluate_count());
    #endif

    if (is_valid_move(move)) {
      printf("Best move: ");
      print_move(move);
      printf("\n");
    } else {
      printf("Game ended\n");
      exit(0);
    }

    // Wait for the user to press enter.
    if (getc(stdin) != '\n') exit(1);

    do_move(&board, move);

    state_t state = get_board_state(&board);
    if (state) {
      print_board(&board, false);
      printf("%s\n", board_state_text(state));
      exit(0);
    }
  }
}

void ai_test(int argc, const char** argv) {
  const char* fen = DEFAULT_BOARD;
  size_t depth = 3;
  if (argc >= 3) {
    fen = argv[2];
    if (argc >= 4) {
      depth = atoi(argv[3]);
    }
  }

  board_t board;
  if (!load_fen(fen, &board)) {
    printf("Could not load FEN.\n");
    exit(1);
  }

  eval_t evaluation;
  evaluate(&board, depth, &evaluation);

  print_eval(evaluation, &board);

  exit(0);
}

int main(int argc, const char** argv) {
  srand(time(NULL));

  int option = 0;
  if (argc >= 2) {
    option = atoi(argv[1]);
  }

  switch (option) {
  case 0: depth_test(argc, argv); break;
  case 1: count_test(argc, argv); break;
  case 2: state_test(argc, argv); break;
  case 3: distance_test(argc, argv); break;
  case 4: iterative_ai_test(argc, argv); break;
  case 5: ai_test(argc, argv); break;
  default:
    break;
      }
}
