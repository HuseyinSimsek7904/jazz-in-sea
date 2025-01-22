#include "ai/move_ordering.h"
#include "ai/cache.h"
#include "ai/eval_t.h"
#include "ai/position_evaluation.h"
#include "ai/transposition_table.h"
#include "board/board_t.h"
#include "board/piece_t.h"
#include "io/pp.h"
#include "move/move_t.h"
#include "state/board_state_t.h"
#include "state/hash_operations.h"

#include <assert.h>
#include <stdbool.h>

typedef struct {
  move_t move;
  eval_t est_eval;
} est_eval_move_t;

int cmp_short_eval_move(const void* a, const void* b) {
  est_eval_move_t a_s = *(est_eval_move_t*) a;
  est_eval_move_t b_s = *(est_eval_move_t*) b;

  return (a_s.est_eval > b_s.est_eval) - (a_s.est_eval < b_s.est_eval);
}

#ifdef MM_OPT_ORDERING
void order_moves(board_state_t* state, ai_cache_t* cache, move_t *moves, size_t length, bool descending) {
  est_eval_move_t eval_moves[256];

  // Copy moves to new buffer to be sorted.
  for (size_t i=0; i<length; i++) {
    move_t move = moves[i];

    piece_t piece = get_piece(state->board, move.from);
    tt_entry_t* entry = get_entry_tt(cache, state->hash ^ get_hash_for_move(state, piece, move));

    eval_t estimate_evaluation;
    if (is_mate(entry->eval)) {
      estimate_evaluation = entry->eval;
    } else {
      // Calculate estimate evaluation using a linear combination of short evaluation and old evaluation.
      eval_t short_evaluation = get_short_move_evaluation(state, cache, move);
      estimate_evaluation =
        entry->eval * entry->depth * cache->est_evaluation_old +
        short_evaluation * cache->est_evaluation_pos;
    }

    // Negate the evaluation score to create the effect of reversing the output.
    eval_moves[i] = (est_eval_move_t){
      .est_eval = descending ? -estimate_evaluation : estimate_evaluation,
      .move = move,
    };
  }

  // Sort the moves according to their short evaluations.
  qsort(eval_moves, length, sizeof(est_eval_move_t), cmp_short_eval_move);

  // Rewrite the sorted moves.
  for (size_t i=0; i<length; i++) {
    moves[i] = eval_moves[i].move;
  }
}
#endif
