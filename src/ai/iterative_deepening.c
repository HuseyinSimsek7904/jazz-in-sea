#include "ai/cache.h"
#include "ai/eval_t.h"
#include "ai/search.h"
#include "ai/iterative_deepening.h"
#include "io/pp.h"
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

  for (size_t depth=1; depth<=max_depth; depth++) {
    move_t killer_moves[256] = { INV_MOVE };
    move_t new_best_moves[256];
    size_t new_best_moves_length = 0;

    eval_t new_evaluation = _evaluate(state,
                                      history,
                                      cache,
                                      depth,
                                      new_best_moves,
                                      &new_best_moves_length,
                                      EVAL_BLACK_MATES,
                                      EVAL_WHITE_MATES,
                                      true,
                                      killer_moves);

    if (new_best_moves_length > 0) {
      *evaluation = new_evaluation;
      *best_moves_length = new_best_moves_length;
      for (size_t i=0; i<new_best_moves_length; i++) {
        best_moves[i] = new_best_moves[i];
      }
    }

    if (new_evaluation == EVAL_INVALID) break;

    io_debug();
    pp_f("debug: best moves for depth %u ", depth);
    pp_moves(best_moves, *best_moves_length);
    pp_f(" -- ");
    pp_eval(new_evaluation, state->board, history);
    pp_f("\n");

    if (is_mate(*evaluation)) {
      io_debug();
      pp_f("debug: reached mate, stopping iterative deepening\n");
      break;
    }
  }

  return NULL;
}
