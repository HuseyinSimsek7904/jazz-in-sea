#ifndef _PIECE_H
#define _PIECE_H

#include <stdbool.h>

#define BLACK  false
#define WHITE  true

#define PAWN   false
#define KNIGHT true

bool is_piece_color(char, bool);
bool is_piece_white(char);
bool is_piece_black(char);

bool is_piece_type(char, bool);
bool is_piece_pawn(char);
bool is_piece_knight(char);

#endif
