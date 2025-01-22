#include "ai/cache.h"
#include "board/pos_t.h"

void setup_cache(ai_cache_t* cache,
                 const int topleft_pawn[4][4],
                 const int topleft_pawn_island[4][4],
                 const int topleft_knight[4][4],
                 const int topleft_knight_island[4][4]) {

  cache->cancel_search = false;

  // Load the advantage tables.
  for (int row=0; row<8; row++) {
    for (int col=0; col<8; col++) {
      int topleft_row = row;
      int topleft_col = col;

      if (row >= 4) topleft_row = 7 - row;
      if (col >= 4) topleft_col = 7 - col;

      pos_t pos = to_position(row, col);

      cache->pawn_adv_table[pos] = topleft_pawn[topleft_row][topleft_col];
      cache->knight_adv_table[pos] = topleft_knight[topleft_row][topleft_col];
      cache->pawn_island_adv_table[pos] = topleft_pawn_island[topleft_row][topleft_col];
      cache->knight_island_adv_table[pos] = topleft_knight_island[topleft_row][topleft_col];
    }
  }

  // For now, load constant values.
  cache->est_evaluation_pos = 1;
  cache->est_evaluation_old = 10;

#ifdef MM_OPT_TRANSPOSITION
  cache->transposition_table = malloc(sizeof(tt_entry_t) * AI_HASHMAP_SIZE);

  // Depth < 0 on a memorized item indicates not set.
  for (size_t i=0; i<AI_HASHMAP_SIZE; i++) {
    (*cache->transposition_table)[i] = (tt_entry_t) {
      .eval = EVAL_INVALID
    };
  }
#endif
}

void free_cache(ai_cache_t* cache) {
#ifdef MM_OPT_TRANSPOSITION
  free(cache->transposition_table);
#endif
}
