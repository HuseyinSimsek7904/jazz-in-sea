#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "move.h"
#include "position.h"

// Print a move.
// The notation for regular moves is: <from_position> > <to_row_or_col_name>
// The notation for capture moves is: <from_position> x <to_row_or_col_name>
// If the piece moved horizontally, <to_row_or_col_name> is the to column name.
// If the piece moved vertically, <to_row_or_col_name> is the to row name.
void print_move(move_t move) {
  print_position(move.from);
  printf("%c", is_valid_pos(move.capture) ? 'x' : '>');

  if (to_col(move.from) == to_col(move.to)) {
    // Column numbers are equal, so the piece moved vertically.
    printf("%c", row_name(to_row(move.to)));
  } else {
    // This should be true according to the normal Cez rules.
    // Because pieces move only horizontally or vertically.
    assert(to_row(move.from) == to_row(move.to));

    printf("%c", col_name(to_col(move.to)));
  }
}
