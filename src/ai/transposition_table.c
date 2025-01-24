#include "ai/transposition_table.h"
#include "ai/cache.h"
#include "ai/eval_t.h"
#include "ai/measure_count.h"
#include "io/pp.h"

#ifdef MM_OPT_TRANSPOSITION

// Return the transposition entry for a board hash.
tt_entry_t* get_entry_tt(ai_cache_t* cache,
                         hash_t hash) {

  return &(*cache->transposition_table)[hash % AI_HASHMAP_SIZE];
}

// Add the board to the transposition table.
void
try_add_tt(ai_cache_t* cache,
           hash_t hash,
           size_t history_size,
           size_t depth,
           eval_t eval,
           node_type_t node_type) {

  // If the eval is an absolute evaluation, convert the depth relative.
  if (is_mate(eval)) {
    eval += eval > 0 ? history_size : -history_size;
    depth = LONG_MAX;
  }

  tt_entry_t* entry = get_entry_tt(cache, hash);

  if (depth <= entry->depth)
    return;

#ifdef MEASURE_EVAL_COUNT
  if (!entry->depth)
    tt_saved_count++;
  else if (hash == entry->hash)
    tt_overwritten_count++;
  else
    tt_rewritten_count++;
#endif

  *entry = (tt_entry_t) {
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
           size_t history_size,
           size_t depth,
           eval_t alpha,
           eval_t beta) {

  // Get the transposition table entry for the board.
  tt_entry_t entry = *get_entry_tt(cache, hash);

  if (entry.hash != hash || entry.eval == EVAL_INVALID || entry.depth < depth) {
    return EVAL_INVALID;
  }

  // If the eval is an absolute evaluation, convert the depth absolute as well.
  if (is_mate(entry.eval)) {
    entry.eval -= entry.eval > 0 ? history_size : -history_size;
  }

  switch (entry.node_type) {
  case EXACT:
    break;
  case LOWER:
    if (entry.eval > alpha) return EVAL_INVALID;
  case UPPER:
    if (entry.eval < beta) return EVAL_INVALID;
  }

  return entry.eval;
}

#endif
