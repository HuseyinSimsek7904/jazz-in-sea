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

int cmp_short_eval_move(const void* a, const void* b) {
  move_eval_pair_t a_s = *(move_eval_pair_t*) a;
  move_eval_pair_t b_s = *(move_eval_pair_t*) b;

  return (a_s.eval > b_s.eval) - (a_s.eval < b_s.eval);
}

bool check_for_killer_move(move_t move, move_t* killer_moves) {
  for (int i=0; is_valid_move(killer_moves[i]); i++) {
    if (compare_move(killer_moves[i], move)) {
      return true;
    }
  }
  return false;
}

void order_moves(board_state_t* state, ai_cache_t* cache, move_t *moves, bool descending, move_t* killer_moves) {
  move_eval_pair_t eval_moves[256];

  // Copy moves to new buffer to be sorted.
  size_t i;
  for (i=0; is_valid_move(moves[i]); i++) {
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

      // Check if this move was a killer move in a sibling.
      if (check_for_killer_move(move, killer_moves)) {
        estimate_evaluation += state->turn ? cache->est_evaluation_killer : -cache->est_evaluation_killer;
      }

    }

    // Negate the evaluation score to create the effect of reversing the output.
    eval_moves[i] = (move_eval_pair_t){
      .eval = descending ? -estimate_evaluation : estimate_evaluation,
      .move = move,
    };
  }

  // Sort the moves according to their short evaluations.
  // i is equal to the length of the array.
  qsort(eval_moves, i, sizeof(move_eval_pair_t), cmp_short_eval_move);

  // Rewrite the sorted moves.
  for (size_t i=0; is_valid_move(moves[i]); i++) {
    moves[i] = eval_moves[i].move;
  }
}
