#ifndef _PIECE_H
#define _PIECE_H

#include <stdbool.h>
#include <sys/types.h>

#define MOD_EMPTY  0
#define MOD_WHITE  4
#define MOD_BLACK  6
#define MOD_PAWN   4
#define MOD_KNIGHT 5

enum {
  EMPTY        = MOD_EMPTY,
  WHITE_PAWN   = MOD_WHITE | MOD_PAWN,
  WHITE_KNIGHT = MOD_WHITE | MOD_KNIGHT,
  BLACK_PAWN   = MOD_BLACK | MOD_PAWN,
  BLACK_KNIGHT = MOD_BLACK | MOD_KNIGHT,
};

typedef u_int8_t piece_t;

static inline char get_piece_color(piece_t piece) { return piece & 6; }
static inline char get_piece_type(piece_t piece) { return piece & 5; }

#endif
