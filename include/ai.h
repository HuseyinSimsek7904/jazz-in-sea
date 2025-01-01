#ifndef _AI_H
#define _AI_H

#include "board.h"
#include "move.h"
#include "position.h"
#include "rules.h"

#include <limits.h>
#include <stddef.h>

#define MAX_AI_MEMO 0x10

typedef struct {
  // Type of evaluation, explains what is the result of the line.
  enum {
    CONTINUE, DRAW, WHITE_WINS, BLACK_WINS, NOT_CALCULATED
  } type;

  // For "CONTINUE" type evaluation, explains how much favorable this position is for white.
  // For "DRAW", "WHITE_WINS", "BLACK_WINS", explains on which move the game will end.
  int strength;
} eval_t;

typedef struct {
#ifdef MEMOIZATION
  struct {
    board_t board;
    size_t depth;
    eval_t eval;
    move_t move;
  } memorized[1 << (8 * sizeof(unsigned short))][MAX_AI_MEMO];
  size_t memorized_size[1 << (8 * sizeof(unsigned short))];
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

void setup_cache(ai_cache_t*);

#ifdef MEMOIZATION
void memorize(ai_cache_t* cache, unsigned short hash, board_t* board, size_t depth, eval_t eval, move_t move);
bool try_remember(ai_cache_t* cache, unsigned short hash, board_t* board, size_t depth, eval_t* eval, move_t* move);
#endif

void setup_adv_tables();

#endif
