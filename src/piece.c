#include <stdbool.h>

#include "piece.h"

// Check the piece color.

bool is_piece_black(char piece) { return piece == 'p' || piece == 'n'; }
bool is_piece_white(char piece) { return piece == 'P' || piece == 'N'; }

bool is_piece_color(char piece, bool color) {
  if (color)
    return is_piece_white(piece);
  else
    return is_piece_black(piece);
}

// Check the piece type.

bool is_piece_pawn(char piece)   { return piece == 'p' || piece == 'P'; }
bool is_piece_knight(char piece) { return piece == 'n' || piece == 'N'; }

bool is_piece_type(char piece, bool type) {
  if (type)
    return is_piece_knight(piece);
  else
    return is_piece_pawn(piece);
}
