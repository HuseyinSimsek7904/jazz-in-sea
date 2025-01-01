#ifndef _AI_H
#define _AI_H

#include "board.h"
#include "move.h"
#include "position.h"
#include "rules.h"

#include <limits.h>
#include <stddef.h>

#define AI_HASHMAP_SIZE 0x20000
#define AI_LL_NODE_SIZE 0x30

typedef struct {
  // Type of evaluation, explains what is the result of the line.
  enum {
    CONTINUE, DRAW, WHITE_WINS, BLACK_WINS, NOT_CALCULATED
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
    move_t move;
  } array[AI_LL_NODE_SIZE];
} ai_cache_node_t;

typedef struct {
#ifdef MEMOIZATION
  ai_cache_node_t* memorized[AI_HASHMAP_SIZE];
#endif
} ai_cache_t;

void print_eval(eval_t, board_t*);

bool compare_eval(bool, eval_t, eval_t);

#ifdef EVALCOUNT
unsigned int get_evaluate_count();
unsigned int get_remember_count();
unsigned int get_game_end_count();
unsigned int get_leaf_count();
#endif

size_t evaluate(board_t*, state_cache_t*, size_t, move_t*, eval_t*);

void setup_cache(ai_cache_t *);
void free_cache(ai_cache_t *);

#ifdef MEMOIZATION
void memorize(ai_cache_t* cache, hash_t hash, board_t* board, size_t depth, eval_t eval, move_t move);
bool try_remember(ai_cache_t* cache, hash_t hash, board_t* board, size_t depth, eval_t* eval, move_t* move);
#endif

void setup_adv_tables();

#endif
