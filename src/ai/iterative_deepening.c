#include "ai/cache.h"
#include "ai/eval_t.h"
#include "ai/search.h"
#include "ai/iterative_deepening.h"
#include "io/pp.h"

void* _id_routine(void* r_args) {
  _id_routine_args_t* args = (_id_routine_args_t*) r_args;

  board_state_t* state = args->state;
  history_t* history = args->history;
  ai_cache_t* cache = args->cache;
  move_t* best_moves = args->best_moves;
  size_t* best_moves_length = args->best_moves_length;
  size_t max_depth = args->max_depth;
  eval_t* evaluation = args->evaluation;

  for (size_t depth=1; depth<=max_depth; depth++) {
    io_debug();
    pp_f("debug: calling _evaluate with depth %u for %s\n", depth, state->turn ? "white" : "black");
    pp_board(state->board, false);

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
                                      true);

    if (new_best_moves_length > 0) {
      *evaluation = new_evaluation;
      *best_moves_length = new_best_moves_length;
      for (size_t i=0; i<new_best_moves_length; i++) {
        best_moves[i] = new_best_moves[i];
      }
    }

    if (new_evaluation == EVAL_INVALID) break;

    io_debug();
    pp_moves(best_moves, *best_moves_length);
    pp_f(" -> ");
    pp_eval(new_evaluation, state->board, history);
    pp_f("\n");
  }

  return NULL;
}
