#ifndef _POSITION_H
#define _POSITION_H

#include <stdbool.h>

#define INV_POSITION 0x88

typedef unsigned char pos_t;

pos_t to_position(int, int);
int to_row(pos_t);
int to_col(pos_t);

int perspective_row(int, bool);
int perspective_col(int, bool);

bool is_valid_pos(pos_t);
bool is_center(pos_t);

#endif
