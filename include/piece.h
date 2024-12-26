#ifndef _PIECE_H
#define _PIECE_H

#include <stdbool.h>

#define BLACK  false
#define WHITE  true

#define PAWN   false
#define KNIGHT true

// Check the piece color.

static inline bool is_piece_black(char piece) { return piece == 'p' || piece == 'n'; }
static inline bool is_piece_white(char piece) { return piece == 'P' || piece == 'N'; }

static inline bool is_piece_color(char piece, bool color) {
  if (color)
    return is_piece_white(piece);
  else
    return is_piece_black(piece);
}

// Check the piece type.

static inline bool is_piece_pawn(char piece)   { return piece == 'p' || piece == 'P'; }
static inline bool is_piece_knight(char piece) { return piece == 'n' || piece == 'N'; }

static inline bool is_piece_type(char piece, bool type) {
  if (type)
    return is_piece_knight(piece);
  else
    return is_piece_pawn(piece);
}

#endif
