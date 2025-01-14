#ifndef _IO_H
#define _IO_H

#include "ai.h"
#include "board.h"
#include "move.h"
#include "position.h"
#include "rules.h"

bool string_to_position(const char*, pos_t*);

bool string_to_move(const char*, board_t, move_t*);

void print_board(board_t, bool);
void print_islands(board_state_t *, bool);

void print_eval(eval_t, board_t, history_t*);
void print_move(move_t);

char col_name(int);
char row_name(int);
void print_position(pos_t);

void print_moves(move_t*, size_t);

#endif
