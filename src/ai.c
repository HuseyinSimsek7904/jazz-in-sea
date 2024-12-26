#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include "ai.h"
#include "board.h"
#include "move.h"
#include "position.h"
#include "piece.h"
#include "rules.h"

const int PAWN_BASE   = -20;
const int KNIGHT_BASE = 700;

// As x increases, distance from center in x decreases.
// Same for y.
const int PAWN_ADV_TABLE[4][4] = {
  {-500, -450, -400, -300},
  {-450, -400, -350, -250},
  {-400, -350, -300, -200},
  {-300, -250, -200, -0  },
};

const int KNIGHT_ADV_TABLE[4][4] = {
  {-300, -250, -200, -100},
  {-250, -200, -150, -60 },
  {-200, -150, -30 , -50 },
  {-100,  -60, -50 , -0  }
};

// Returns the copy of eval from the opponent's POV.
// WHITE_WINS => BLACK_WINS
// BLACK_WINS => WHITE_WINS
// DRAW => DRAW
// CONTINUE(x) => CONTINUE(-x)
eval_t _switch_eval_turn(eval_t eval) {
  switch (eval.type) {
  case WHITE_WINS:
    return (eval_t) { .type=BLACK_WINS, .strength=eval.strength };
  case BLACK_WINS:
    return (eval_t) { .type=WHITE_WINS, .strength=eval.strength };
  case CONTINUE:
    return (eval_t){.type = CONTINUE, .strength = -eval.strength};

  case DRAW:
  case NOT_CALCULATED:
    return eval;
  }
  return (eval_t) {};
}

// Compare eval1 and eval2 by whether they are favorable for the player.
// <0 => eval2 is more favorable than eval1
// =0 => neither is more favorable
// >0 => eval1 is more favorable than eval2
// NOT_CALCULATED evals can not be compared.
int compare_favor(eval_t eval1, eval_t eval2, bool turn) {
  // Convert the eval structs to whites POV to simplify the function.
  // This way, for all player these can be used to mean the same thing.
  // WHITE_WINS => WINS
  // BLACK_WINS => LOSES
  if (!turn) {
    eval1 = _switch_eval_turn(eval1);
    eval2 = _switch_eval_turn(eval2);
  }

  switch (eval1.type) {
  case WHITE_WINS:
    // Unless eval2 is WHITE_WINS, always eval1.
    // Otherwise compare the depths.
    return eval2.type != WHITE_WINS ? 1 : eval2.strength - eval1.strength;

  case BLACK_WINS:
    // Unless eval2 is BLACK_WINS, always eval2.
    // Otherwise compare the depths.
    return eval2.type != BLACK_WINS ? -1 : eval2.strength - eval1.strength;

  case DRAW:
    switch (eval2.type) {
      // If eval2 is WHITE_WINS, always eval2.
    case WHITE_WINS: return -1;
      // If eval2 is BLACK_WINS, always eval1.
    case BLACK_WINS: return 1;
      // If eval2 is DRAW, check for moves.
      // This comparison is not actually necessary, we are just trying to make the AI choose the simpler lines.
    case DRAW: return eval2.strength - eval1.strength;
      // If eval2 is CONTINUE, check for the evaluation.
    case CONTINUE: return eval2.strength;

    case NOT_CALCULATED:
      assert(false);
      break;
    }

  case CONTINUE:
    switch (eval2.type) {
      // If eval2 is WHITE_WINS, always eval2.
    case WHITE_WINS: return -1;
      // If eval2 is BLACK_WINS, always eval1.
    case BLACK_WINS: return 1;
      // If eval2 is DRAW, check for the evaluation.
    case DRAW: return eval1.strength;
      // If eval2 is CONTINUE, check for the evaluation difference.
    case CONTINUE: return eval1.strength - eval2.strength;

    case NOT_CALCULATED:
      assert(false);
      break;
    }

  case NOT_CALCULATED:
    assert(false);
    break;
  }

  return 0;
}

static inline int get_delta_eval(board_t* board, move_t move) {
  int delta_evaluation = 0;

  // Add the advantage of the piece.
  // It is a "good" thing that we take the opponent's piece as these pieces have a base value.
  // Losing a piece will make you lose that base value.
  if (is_valid_pos(move.capture)) {
    if (is_piece_knight(move.capture_piece))
      delta_evaluation += KNIGHT_BASE;
    else {
      assert(is_piece_pawn(move.capture_piece));
      delta_evaluation += PAWN_BASE;
    }
  }

  // Add the advantage difference of the from and to positions.
  char piece = get_piece(board, move.from);
  if (is_piece_knight(piece)) {
    delta_evaluation += knight_pos_adv(move.to) - knight_pos_adv(move.from);
  } else {
    assert(is_piece_pawn(piece));
    delta_evaluation += pawn_pos_adv(move.to) - pawn_pos_adv(move.from);
  }

  return board->turn ? delta_evaluation : -delta_evaluation;
}

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

int pawn_pos_adv(pos_t pos) {
  int col = to_col(pos);
  int row = to_row(pos);
  if (col >= 4) col = 7 - col;
  if (row >= 4) row = 7 - row;
  return PAWN_ADV_TABLE[row][col];
}

int knight_pos_adv(pos_t pos) {
  int col = to_col(pos);
  int row = to_row(pos);
  if (col >= 4) col = 7 - col;
  if (row >= 4) row = 7 - row;
  return KNIGHT_ADV_TABLE[row][col];
}

#ifdef EVALCOUNT
unsigned int evaluate_count = 0;

unsigned int get_evaluate_count() {
  return evaluate_count;
}
#endif

// Find the best continuing moves available and its evaluation.
size_t
_evaluate(board_t* board,
          state_cache_t* state,
          size_t max_depth,
          move_t* best_moves,
          eval_t* evaluation,
          eval_t alpha,
          eval_t beta,
          bool starting_move) {

  // Can be used to debug whilst trying to optimise the evaluate function.
  #ifdef EVALCOUNT
  evaluate_count++;
  #endif

  // Check for the board state.
  // If the game should not continue, return the evaluation.
  switch (state->status & 0x30) {
  case 0x10:
    *evaluation = (eval_t) { .type=DRAW,       .strength=board->move_count };
    return 0;
  case 0x20:
    *evaluation = (eval_t) { .type=WHITE_WINS, .strength=board->move_count };
    return 0;
  case 0x30:
    *evaluation = (eval_t) { .type=BLACK_WINS, .strength=board->move_count };
    return 0;
  }

  // Check if we reached the end of the best_line buffer.
  // If so, just return the evaluation.
  if (!max_depth) {
    *evaluation = (eval_t) { .type=CONTINUE, .strength=0 };
    return 0;
  }

  move_t moves[256];
  int moves_length = generate_moves(board, moves);

  // There must be at least 1 moves, otherwise we should not pass the state check step.
  assert(moves_length);

  // If this move is the first move and if there are only one possible moves, return the only move.
  // No need to search recursively.
  if (starting_move && moves_length == 1) {
    *evaluation = (eval_t) { .type=NOT_CALCULATED };
    *best_moves = moves[0];
    return 1;
  }

  size_t found_moves = 0;

  // Loop through all of the available moves except the first, and recursively get the next moves.
  for (int i=0; i<moves_length; i++) {
    // If the move was a capture move, do not decrement the depth.
    move_t move = moves[i];
    eval_t new_evaluation;
    size_t new_depth = max_depth - 1;
    move_t new_moves[256];

    do_move(board, state, move);
    _evaluate(board, state, new_depth, new_moves, &new_evaluation, alpha, beta, false);
    undo_move(board, state, move);

    // If the evaluation type was CONTINUE, then add the move delta evaluation.
    if (new_evaluation.type == CONTINUE)
      new_evaluation.strength += get_delta_eval(board, move);

    // If this move is not the first move, compare this move with the best move.
    if (found_moves) {
      // Compare this move and the old best move.
      int cmp = compare_favor(new_evaluation, *evaluation, board->turn);

      if (cmp < 0) {
        // If this move is worse than the found moves, continue.
        continue;

      } else if (cmp == 0) {
        // If this move is equally as good as the best move, add this move to the list.
        best_moves[found_moves++] = move;
        continue;
      }
    }

    *evaluation = new_evaluation;
    found_moves = 1;
    *best_moves = move;

    // If found a move better than beta or alpha, break.
    if (compare_favor(new_evaluation, board->turn ? beta : alpha, board->turn) > 0) {
      break;
    }

    // Update the limit variables alpha and beta.
    if (board->turn) {
      if (compare_favor(new_evaluation, alpha, board->turn) > 0)
        alpha = new_evaluation;
    } else {
      if (compare_favor(new_evaluation, beta, board->turn) > 0)
        beta = new_evaluation;
    }
  }

  return found_moves;
}

size_t evaluate(board_t* board, state_cache_t* state, size_t max_depth, move_t* moves, eval_t* evaluation) {
#ifdef EVALCOUNT
  evaluate_count = 0;
#endif

  size_t length = _evaluate(board,
                            state,
                            max_depth,
                            moves,
                            evaluation,
                            (eval_t) { .type=BLACK_WINS, .strength=0 },  // best possible evaluation for black
                            (eval_t) { .type=WHITE_WINS, .strength=0 },  // best possible evaluation for white
                            true);

  return length;
}
