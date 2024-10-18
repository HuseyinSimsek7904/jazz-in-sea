#pragma once

#include "board.h"
#include "move.h"
#include "position.h"

#include <limits.h>
#include <stddef.h>

#define DRAW_EVAL 0
#define WHITE_WON_EVAL INT_MAX
#define BLACK_WON_EVAL INT_MIN

int pawn_dist_to_center(pos_t);
int knight_dist_to_center(pos_t);

int evaluate_board(board_t*);
move_t evaluate(board_t*, size_t, int*);
