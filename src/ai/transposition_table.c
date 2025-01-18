#include "ai/transposition_table.h"
#include "ai/measure_count.h"
#include "io/pp.h"

#ifdef MM_OPT_TRANSPOSITION

// Add the board to the transposition table.
void
try_add_tt(ai_cache_t* cache,
           hash_t hash,
           history_t* history,
           size_t depth,
           eval_t eval,
           node_type_t node_type) {

  // If the eval is an absolute evaluation, convert the depth relative.
  if (is_mate(eval)) {
    eval += eval > 0 ? history->size : -history->size;
    depth = LONG_MAX;
  }

  tt_entry_t* memorized = &(*cache->transposition_table)[hash % AI_HASHMAP_SIZE];

  if (depth <= memorized->depth)
    return;

#ifdef MEASURE_EVAL_COUNT
  if (!memorized->depth)
    tt_saved_count++;
  else if (hash == memorized->hash)
    tt_overwritten_count++;
  else
    tt_rewritten_count++;
#endif

  *memorized = (tt_entry_t) {
    .hash = hash,
    .depth = depth,
    .eval = eval,
    .node_type = node_type,
  };
}

// Get if the board was saved for memoization before.
eval_t
try_find_tt(ai_cache_t* cache,
           hash_t hash,
           history_t* history,
           size_t depth,
           eval_t alpha,
           eval_t beta) {

  tt_entry_t memorized = (*cache->transposition_table)[hash % AI_HASHMAP_SIZE];

  if (memorized.eval == EVAL_INVALID || memorized.depth < depth || memorized.hash != hash) {
    return EVAL_INVALID;
  }

  // If the eval is an absolute evaluation, convert the depth absolute as well.
  if (is_mate(memorized.eval)) {
    memorized.eval -= memorized.eval > 0 ? history->size : -history->size;
  }

  switch (memorized.node_type) {
  case EXACT:
    break;
  case LOWER:
    if (memorized.eval > alpha) return EVAL_INVALID;
  case UPPER:
    if (memorized.eval < beta) return EVAL_INVALID;
  }

  return memorized.eval;
}

#endif
