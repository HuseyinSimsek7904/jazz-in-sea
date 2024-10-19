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

// Can cause the AI to always play moves that guarantee mates but never actually
// mate. Could be a fun way to play against the game actually.
const bool quick_mate_optimisation = false;

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

  case INVALID:
    assert(false);
  case DRAW:
  case NOT_CALCULATED:
    return eval;
  }
  return (eval_t) {};
}

void print_eval(eval_t eval, board_t* board) {
  switch (eval.type) {
  case WHITE_WINS:
    printf("white mates in %d\n", eval.strength - board->move_count);
    break;
  case BLACK_WINS:
    printf("black mates in %d\n", eval.strength - board->move_count);
    break;
  case DRAW:
    printf("draw in %u\n", eval.strength);
    break;
  case CONTINUE:
    printf("continue with advantage %i\n", eval.strength);
    break;
  case NOT_CALCULATED:
    printf("not calculated\n");
    break;
  case INVALID:
    assert(false);
    printf("<invalid eval>\n");
    break;
  }
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
    case INVALID:
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
    case INVALID:
      assert(false); break;
    }

  case NOT_CALCULATED:
  case INVALID:
    assert(false);
    break;
  }

  return 0;
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

int _pawn_pos_adv(pos_t pos) {
  int dist = pawn_dist_to_center(pos);
  return -dist * dist;
}

int _knight_pos_adv(pos_t pos) {
  int dist = knight_dist_to_center(pos);
  return -dist * dist;
}


#ifdef EVALCOUNT
unsigned int evaluate_count = 0;

unsigned int get_evaluate_count() {
  return evaluate_count;
}
#endif

// Evaluate the board.
eval_t evaluate_board(board_t* board) {
  // Check for the board state.
  state_t state = get_board_state(board);
  switch (state & 0x30) {
  case 1: return (eval_t) { .type=DRAW,       .strength=board->move_count };
  case 2: return (eval_t) { .type=WHITE_WINS, .strength=board->move_count };
  case 3: return (eval_t) { .type=BLACK_WINS, .strength=board->move_count };
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
        piece_eval = PAWN_BASE + _pawn_pos_adv(pos);
      else
        piece_eval = KNIGHT_BASE + _knight_pos_adv(pos);

      // If the piece is black, then negate the evaluation.
      if (is_piece_black(piece))
        piece_eval *= -1;

      eval += piece_eval;
    }
  }

  // Return the generated strength object as a continue evaluation.
  return (eval_t) { .type=CONTINUE, .strength=eval};
}


// Find the best continuing move available and its evaluation.
move_t
_evaluate(board_t* board,
          size_t max_depth,
          eval_t* evaluation,
          eval_t alpha,
          eval_t beta,
          bool starting_move) {

  // Can be used to debug whilst trying to optimise the evaluate function.
  #ifdef EVALCOUNT
  evaluate_count++;
  #endif

  // Check if we reached the end of the best_line buffer.
  // If so, just return the evaluation.
  if (!max_depth) {
    *evaluation = evaluate_board(board);
    assert(evaluation->type != INVALID);
    return INV_MOVE;
  }

  // Check for the board state.
  // If the game should not continue, return the evaluation.
  state_t state = get_board_state(board);
  switch (state & 0x30) {
  case 0x10:
    *evaluation = (eval_t) { .type=DRAW,       .strength=board->move_count };
    return INV_MOVE;
  case 0x20:
    *evaluation = (eval_t) { .type=WHITE_WINS, .strength=board->move_count };
    return INV_MOVE;
  case 0x30:
    *evaluation = (eval_t) { .type=BLACK_WINS, .strength=board->move_count };
    return INV_MOVE;
  }

  move_t moves[256];
  int length = generate_moves(board, moves);

  // There must be at least 1 moves, otherwise we should not pass the state check step.
  assert(length);

  // If this move is the first move and if there are only one possible moves, return the only move.
  // No need to search recursively.
  if (starting_move && length == 1) {
    *evaluation = (eval_t) { .type=NOT_CALCULATED };
    return moves[0];
  }

  move_t best_move;
  *evaluation = (eval_t) { .type=INVALID };

  // Loop through all of the available moves except the first, and recursively get the next moves.
  for (int i=0; i<length; i++) {
    move_t move = moves[i];
    do_move(board, move);
    eval_t new_evaluation;

    // If the move was a capture move, do not decrement the depth.
    size_t new_depth = max_depth - (is_valid_pos(move.capture) ? 0 : 1);

    _evaluate(board, new_depth, &new_evaluation, alpha, beta, false);
    undo_move(board, move);

    // If the found move is better than the latest best move, update it.
    if (evaluation->type == INVALID || compare_favor(new_evaluation, *evaluation, board->turn) > 0) {
      *evaluation = new_evaluation;
      best_move = move;

      // If found a mate for the current player, select this move automatically and stop iterating.
      if (quick_mate_optimisation &&
          ((board->turn && evaluation->type == WHITE_WINS) ||
           (!board->turn && evaluation->type == BLACK_WINS)))
        return move;
    }

    // Update the limit variables alpha and beta.
    if (board->turn) {
      if (compare_favor(new_evaluation, alpha, board->turn))
        alpha = new_evaluation;
    } else {
      if (compare_favor(new_evaluation, beta, board->turn))
        beta = new_evaluation;
    }

    // If found a move better than beta or alpha, break.
    if (compare_favor(new_evaluation, board->turn ? beta : alpha, board->turn) > 0)
      break;
  }

  assert(evaluation->type != INVALID);
  return best_move;
}

move_t evaluate(board_t* board, size_t max_depth, eval_t* evaluation) {
  #ifdef EVALCOUNT
  evaluate_count = 0;
  #endif

  return _evaluate(board,
                   max_depth,
                   evaluation,
                   (eval_t) { .type=BLACK_WINS, .strength=0 },  // best possible evaluation for black
                   (eval_t) { .type=WHITE_WINS, .strength=0 },  // best possible evaluation for white
                   true);
}
