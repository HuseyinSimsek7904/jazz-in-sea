#ifndef _AI_H
#define _AI_H

#include "board.h"
#include "move.h"
#include "position.h"
#include "rules.h"

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>

#define AI_HASHMAP_SIZE 0x20000
#define AI_LL_NODE_SIZE 0x30

typedef struct {
  // Type of evaluation, explains what is the result of the line.
  enum {
    CONTINUE, WHITE_WINS, BLACK_WINS, NOT_CALCULATED
  } type;

  // For "CONTINUE" type evaluation, explains how much favorable this position is for white.
  // For "DRAW", "WHITE_WINS", "BLACK_WINS", explains on which move the game will end.
  int strength;
} eval_t;

typedef struct ai_cache_node_t {
  struct ai_cache_node_t* next;

  size_t size;
  struct memorized_t {
    board_t board;
    hash_t hash;
    size_t depth;
    eval_t eval;
    // TODO: Do not store the move.
    move_t move;
  } array[AI_LL_NODE_SIZE];
} ai_cache_node_t;

typedef struct {
  int pawn_adv_table[256];
  int pawn_island_adv_table[256];
  int knight_adv_table[256];
  int knight_island_adv_table[256];

#ifdef MM_OPT_MEMOIZATION
  ai_cache_node_t* memorized[AI_HASHMAP_SIZE];
#endif
} ai_cache_t;

int compare_eval(eval_t, eval_t);
int compare_eval_by(eval_t, eval_t, bool);

#ifdef MEASURE_EVAL_COUNT
unsigned int get_evaluate_count();
unsigned int get_game_end_count();
unsigned int get_leaf_count();

#ifdef MM_OPT_MEMOIZATION
unsigned int get_remember_count();
unsigned int get_saved_count();
#endif

#ifdef MM_OPT_AB_PRUNING
unsigned int get_ab_branch_cut_count();
#endif

#endif

eval_t evaluate(board_t*, state_cache_t*, history_t*, size_t, move_t*, size_t*);

void setup_cache(ai_cache_t *,
                 const int[4][4],
                 const int[4][4],
                 const int[4][4],
                 const int[4][4]);
void free_cache(ai_cache_t *);

#ifdef MM_OPT_MEMOIZATION
void memorize(ai_cache_t*, hash_t, board_t*, history_t*, size_t, eval_t, move_t);
bool try_remember(ai_cache_t*, hash_t, board_t*, history_t*, size_t, eval_t*, move_t*);
#endif

#endif
