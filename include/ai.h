#ifndef _AI_H
#define _AI_H

#include "board.h"
#include "move.h"
#include "position.h"
#include "rules.h"

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#define AI_HASHMAP_SIZE 0x20000

typedef int eval_t;

#define EVAL_INVALID     INT_MIN
#define EVAL_WHITE_MATES INT_MAX
#define EVAL_BLACK_MATES INT_MIN + 1

static inline bool is_mate(eval_t eval) { return abs(eval) > 1000000; }
static inline int mate_depth(eval_t eval) { return EVAL_WHITE_MATES - abs(eval); }

typedef enum { EXACT, LOWER, UPPER } node_type_t;

typedef struct {
  hash_t hash;
  size_t depth;
  eval_t eval;
  node_type_t node_type;
} memorized_t;

typedef struct {
  int pawn_adv_table[256];
  int pawn_island_adv_table[256];
  int knight_adv_table[256];
  int knight_island_adv_table[256];

#ifdef MM_OPT_MEMOIZATION
  memorized_t (*memorized)[AI_HASHMAP_SIZE];
#endif
} ai_cache_t;

#ifdef MEASURE_EVAL_COUNT
unsigned int get_evaluate_count();
unsigned int get_ab_branch_cut_count();
unsigned int get_game_end_count();
unsigned int get_leaf_count();

#ifdef MM_OPT_MEMOIZATION
unsigned int get_tt_remember_count();
unsigned int get_tt_saved_count();
unsigned int get_tt_overwritten_count();
unsigned int get_tt_rewritten_count();
#endif

#endif

eval_t evaluate(board_state_t*, history_t*, size_t, move_t*, size_t*);

void setup_cache(ai_cache_t *,
                 const int[4][4],
                 const int[4][4],
                 const int[4][4],
                 const int[4][4]);
void free_cache(ai_cache_t *);

#ifdef MM_OPT_MEMOIZATION
void memorize(ai_cache_t*, hash_t, history_t*, size_t, eval_t, node_type_t);
eval_t try_remember(ai_cache_t*, hash_t, history_t*, size_t, eval_t, eval_t);
#endif

#endif
