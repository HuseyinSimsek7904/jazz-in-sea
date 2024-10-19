#ifndef _AI_H
#define _AI_H

#include "board.h"
#include "move.h"
#include "position.h"

#include <limits.h>
#include <stddef.h>

typedef struct {
  // Type of evaluation, explains what is the result of the line.
  enum {
    CONTINUE, DRAW, WHITE_WINS, BLACK_WINS, INVALID, NOT_CALCULATED
  } type;

  // For "CONTINUE" type evaluation, explains how much favorable this position is for white.
  // For "DRAW", "WHITE_WINS", "BLACK_WINS", explains on which move the game will end.
  int strength;
} eval_t;

void print_eval(eval_t, board_t*);

bool compare_eval(bool, eval_t, eval_t);

int pawn_dist_to_center(pos_t);
int knight_dist_to_center(pos_t);

#ifdef EVALCOUNT
unsigned int get_evaluate_count();
#endif

eval_t evaluate_board(board_t*);
move_t evaluate(board_t*, size_t, eval_t*);

#endif
