#ifndef _IO_H
#define _IO_H

#include "board.h"
#include "move.h"

void print_board(board_t *, bool);

void print_move(move_t);

char col_name(int);
char row_name(int);
void print_position(pos_t);

#endif
