#include <assert.h>
#include <stdio.h>

#include "ai.h"
#include "board.h"
#include "io.h"
#include "move.h"

// Convert from regular row to perspective row or vice verca.
// Perspective row means the row that the player sees.
int perspective_row(int row, bool player) { return player ? row : 7 - row; }

// Convert from regular column to perspective column or vice versa.
// Perspective col means the column that the player sees.
int perspective_col(int col, bool player) { return player ? 7 - col : col; }

// Get the column name from a column number.
// aka 0->a, 1->b ...  7->h
char col_name(int col) { return 'a' + col; }

// Get the row name from a row number.
// aka 0->1, 1->2, ... 7->8
char row_name(int row) { return '1' + row; }

// Print a position.
// The notation is: <column-name><row-number>
void print_position(pos_t pos) {
  printf("%c%c", col_name(to_col(pos)), row_name(to_row(pos)));
}

// Print a move.
// The notation for regular moves is: <from_position> > <to_row_or_col_name>
// The notation for capture moves is: <from_position> x <to_row_or_col_name>
// If the piece moved horizontally, <to_row_or_col_name> is the to column name.
// If the piece moved vertically, <to_row_or_col_name> is the to row name.
void print_move(move_t move) {
  // Check if the move is valid.
  if (!is_valid_move(move)) {
    printf("<invalid move>");
    return;
  }

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

const char *TOP_ROW = " abcdefgh\n";
const char *TOP_ROW_INV = " hgfedcba\n";

const char WHITE_CELL = '_';
const char BLACK_CELL = ' ';
const char WHITE_CELL_CENTER = '.';
const char BLACK_CELL_CENTER = '.';
const char WHITE_PAWN = 'P';
const char WHITE_KNIGHT = 'N';
const char BLACK_PAWN = 'p';
const char BLACK_KNIGHT = 'n';
const char ERROR = 'E';

void print_board(board_t *board, bool player) {
  printf("%s", player ? TOP_ROW_INV : TOP_ROW);

  for (int prow = 0; prow < 8; prow++) {
    int row = perspective_row(prow, player);
    printf("%c", row_name(row));

    for (int pcol = 0; pcol < 8; pcol++) {
      int col = perspective_col(pcol, player);

      pos_t position = to_position(row, col);
      char piece = get_piece(board, position);

      char character;
      switch (piece) {
      case ' ':
        if (is_center(position)) {
          character = (col + row) % 2 ? WHITE_CELL_CENTER : BLACK_CELL_CENTER;
        } else {
          character = (col + row) % 2 ? WHITE_CELL : BLACK_CELL;
        }
        break;

      case 'P':
        character = WHITE_PAWN;
        break;
      case 'N':
        character = WHITE_KNIGHT;
        break;
      case 'p':
        character = BLACK_PAWN;
        break;
      case 'n':
        character = BLACK_KNIGHT;
        break;
      default:
        character = ERROR;
        break;
      }

      printf("%c", character);
    }
    printf("\n");
  }
}

void print_eval(eval_t eval, board_t *board) {
  switch (eval.type) {
  case WHITE_WINS:
    printf("white mates in %d\n", eval.strength - board->move_count);
    break;
  case BLACK_WINS:
    printf("black mates in %d\n", eval.strength - board->move_count);
    break;
  case DRAW:
    printf("draw in %u\n", eval.strength);
    break;
  case CONTINUE:
    printf("continue with advantage %i\n", eval.strength);
    break;
  case NOT_CALCULATED:
    printf("not calculated\n");
    break;
  case INVALID:
    assert(false);
    printf("<invalid eval>\n");
    break;
  }
}
