#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "ai.h"
#include "board.h"
#include "move.h"
#include "position.h"
#include "piece.h"
#include "rules.h"

const int PAWN_BASE = 4;
const int KNIGHT_BASE = 2;

// Returns how many regular pawn moves it would take for the pawn to walk to the
// center of the board.
int pawn_dist_to_center(pos_t pos) {
  int col = to_col(pos);
  int row = to_row(pos);
  return (col <= 3 ? 3 - col : col - 4) + (row <= 3 ? 3 - row : row - 4);
}

// Returns how many regular knight moves it would take for the pawn to walk to
// the center of the board.
int knight_dist_to_center(pos_t pos) {
  int col = to_col(pos);
  int row = to_row(pos);
  return (col <= 3 ? (4 - col) / 2 : (col - 3) / 2) + (row <= 3 ? (4 - row) / 2 : (row - 3) / 2);
}

// Evaluate the board.
int evaluate_board(board_t* board) {
  // Check for the board state.
  state_t state = get_board_state(board);
  switch (state & 0x30) {
  case 1: return 0;
  case 2: return WHITE_WON_EVAL;
  case 3: return BLACK_WON_EVAL;
  }

  // If the game did not end yet, look for the positions of pieces.
  int eval = 0;
  for (int row=0; row<8; row++) {
    for (int col=0; col<8; col++) {
      pos_t pos = to_position(row, col);
      char piece = get_piece(board, pos);

      if (piece == ' ') continue;

      // Calculate the evaluation for the piece.
      int piece_eval;
      if (is_piece_pawn(piece))
        piece_eval = PAWN_BASE - pawn_dist_to_center(pos);
      else
        piece_eval = KNIGHT_BASE - knight_dist_to_center(pos);

      // If the piece is black, then negate the evaluation.
      if (is_piece_black(piece))
        piece_eval *= -1;

      eval += piece_eval;
    }
  }
  return eval;
}


// Find the best continuing move available and its evaluation.
move_t _evaluate(board_t* board, size_t max_depth, int* evaluation, bool starting_move) {
  // Check if we reached the end of the best_line buffer.
  // If so, just return the evaluation.
  if (!max_depth) {
    *evaluation = evaluate_board(board);
    return INV_MOVE;
  }

  // Check for the board state.
  // If the game should not continue, return the evaluation.
  state_t state = get_board_state(board);
  switch (state & 0x30) {
  case 0x10:
    *evaluation = DRAW_EVAL;
    return INV_MOVE;
  case 0x20:
    *evaluation = WHITE_WON_EVAL;
    return INV_MOVE;
  case 0x30:
    *evaluation = BLACK_WON_EVAL;
    return INV_MOVE;
  }

  move_t moves[256];
  int length = generate_moves(board, moves);

  // There must be at least 1 moves, otherwise we should not pass the state check step.
  assert(length);

  // If this move is the first move and if there are only one possible moves, return the only move.
  // No need to search recursively.
  if (length == 1) {
    *evaluation = 0;
    return moves[0];
  }

  // Calculate the starting evaluation using the first move.
  move_t best_move = moves[0];
  do_move(board, moves[0]);
  evaluate(board, max_depth - 1, evaluation);
  undo_move(board, moves[0]);

  // If found a mate for the current player, select this move automatically and stop iterating.
  if ((board->turn && *evaluation == INT_MAX) || (!board->turn && *evaluation == INT_MIN))
    return moves[0];

  // Loop through all of the available moves except the first, and recursively get the next moves.
  for (int i=1; i<length; i++) {
    move_t move = moves[i];
    do_move(board, move);
    int new_evaluation;

    // If the move was a capture move, do not decrement the depth.
    size_t new_depth = max_depth - (is_valid_pos(move.capture) ? 0 : 1);

    evaluate(board, new_depth, &new_evaluation);
    undo_move(board, move);

    // If the found move is better than the latest best move, update it.
    if ((board->turn && new_evaluation >= *evaluation) || (!board->turn && new_evaluation <= *evaluation)) {
      *evaluation = new_evaluation;
      best_move = move;
    }

    // If found a mate for the current player, select this move automatically and stop iterating.
    if ((board->turn && new_evaluation == INT_MAX) || (!board->turn && new_evaluation == INT_MIN))
      return move;
  }

  return best_move;
}

move_t evaluate(board_t* board, size_t max_depth, int* evaluation) {
  return _evaluate(board, max_depth, evaluation, true);
}
