#ifndef _IO_H
#define _IO_H

#include "board.h"
#include "move.h"
#include "position.h"
#include "rules.h"

bool string_to_position(const char*, pos_t*);

bool string_to_move(const char*, board_t* board, move_t*);

void print_board(board_t *, bool);
void print_islands(board_t*, state_cache_t*, bool);

void print_move(move_t);

char col_name(int);
char row_name(int);
void print_position(pos_t);

void print_moves(move_t*, size_t);

#endif
