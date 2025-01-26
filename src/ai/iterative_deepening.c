#include "ai/cache.h"
#include "ai/eval_t.h"
#include "ai/search.h"
#include "ai/iterative_deepening.h"
#include "ai/move_ordering.h"
#include "io/pp.h"
#include "move/generation.h"
#include "move/make_move.h"
#include "move/move_t.h"

void* _id_routine(void* r_args) {
  _id_routine_args_t* args = (_id_routine_args_t*) r_args;

  board_state_t* state = args->state;
  history_t* history = args->history;
  ai_cache_t* cache = args->cache;
  move_t* best_moves = args->best_moves;
  size_t* best_moves_length = args->best_moves_length;
  size_t max_depth = args->max_depth;
  eval_t* evaluation = args->evaluation;

  // Dump the board information for debugging.
  io_debug();
  pp_f("debug: calling _evaluate for color %s\n", state->turn ? "white" : "black");
  pp_board(state->board, false);

  move_t moves[256];
  eval_t evals[256];
  size_t moves_length = generate_moves(state, moves);

  // If there are no moves available, return draw by no moves.
  if (moves_length == 0) {
    *best_moves_length = 0;
    *evaluation = EVAL_INVALID;
    return NULL;
  }

  // If there is only one move available, return that only move.
  if (moves_length == 1) {
    best_moves[0] = moves[0];
    *best_moves_length = 1;
    *evaluation = EVAL_INVALID;
    return NULL;
  }

  // Reset all of the evals.
  for (size_t i=0; i<moves_length; i++) {
    evals[i] = EVAL_INVALID;
  }

  move_t killer_moves[256] = { INV_MOVE };

  // Iterate depths from 1 to max_depth.
  for (size_t depth=1; depth<=max_depth; depth++) {
    order_moves(state, cache, moves, moves_length, state->turn, killer_moves);

    // Iterate all moves.
    for (size_t i=0; i<moves_length; i++) {
      // TODO: Implement ignoring absolute evaluations.
      /* // Ignore moves that are already known to be mates. */
      /* if (evals[i] != EVAL_INVALID && is_mate(evals[i])) */
      /*   continue; */

      do_move(state, history, moves[i]);

      eval_t move_eval = _evaluate(state,
                                   history,
                                   cache,
                                   depth - 1,
                                   EVAL_BLACK_MATES,
                                   EVAL_WHITE_MATES,
                                   killer_moves);

      undo_last_move(state, history);

      if (move_eval == EVAL_INVALID) {
        io_debug();
        pp_f("[search canceled]\n");
        return NULL;
      }

      evals[i] = move_eval;
    }

    // Print the move evaluation scores.
    io_debug();
    pp_f("depth=%u, { ", depth);
    for (size_t i=0; i<moves_length; i++) {
      pp_move(moves[i]);
      pp_f(": ");
      pp_eval(evals[i], state->board, history);
      pp_f(", ");
    }
    pp_f("}\n");

    // Select the best moves.
    *evaluation = state->turn ? EVAL_BLACK_MATES : EVAL_WHITE_MATES;
    *best_moves_length = 0;

    for (size_t i=0; i<moves_length; i++) {
      if (evals[i] == *evaluation) {
        best_moves[(*best_moves_length)++] = moves[i];
      } else if ((evals[i] < *evaluation) ^ state->turn) {
        *best_moves_length = 1;
        best_moves[0] = moves[i];
        *evaluation = evals[i];
      }
    }

    if (is_mate(*evaluation)) {
      io_debug();
      pp_f("debug: reached unavoidable mate, stopping iterative deepening\n");
      break;
    }
  }

  return NULL;
}
