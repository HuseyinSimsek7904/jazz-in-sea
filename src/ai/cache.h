#ifndef _AI_CACHE_H
#define _AI_CACHE_H

#include "ai/eval_t.h"
#include "board/hash_t.h"

#define AI_HASHMAP_SIZE 0x20000

#ifdef MM_OPT_TRANSPOSITION
typedef enum { EXACT, LOWER, UPPER } node_type_t;

typedef struct {
  hash_t hash;
  size_t depth;
  eval_t eval;
  node_type_t node_type;
} tt_entry_t;
#endif

typedef struct {
  int pawn_adv_table[256];
  int pawn_island_adv_table[256];
  int knight_adv_table[256];
  int knight_island_adv_table[256];

#ifdef MM_OPT_TRANSPOSITION
  tt_entry_t (*transposition_table)[AI_HASHMAP_SIZE];
#endif
} ai_cache_t;

void setup_cache(ai_cache_t *,
                 const int[4][4],
                 const int[4][4],
                 const int[4][4],
                 const int[4][4]);
void free_cache(ai_cache_t *);

#endif
