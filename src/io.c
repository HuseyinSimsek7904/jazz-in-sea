#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "ai.h"
#include "board.h"
#include "io.h"
#include "move.h"
#include "piece.h"
#include "position.h"
#include "rules.h"

// Try to convert a string to a position.
bool string_to_position(const char* s, pos_t* pos) {
  char col_name = *s++;
  if (col_name < 'a' || col_name > 'h') return false;

  char row_name = *s++;
  if (row_name < '1' || row_name > '8') return false;

  if (*s != '\0') return false;

  *pos = to_position(row_name - '1', col_name - 'a');
  return true;
}

// Given the delta multiplier of a move's capture position, return the move's regular delta multiplier.
int _capture_delta_to_regular_delta(int dist) {
  switch (dist) {
  case -3: return -2;
  case -2: return -1;
  case  2: return  1;
  case  3: return  2;
  default:
    assert(false);
    return  0;
  }
}

// Try to convert a string to a move.
bool string_to_move(const char* s, board_t* board, move_t* move) {
  char row_name = *s++;
  if (row_name < 'a' || row_name > 'h') return false;

  char col_name = *s++;
  if (col_name < '1' || col_name > '8') return false;

  char move_type = *s++;
  if (move_type != '>' && move_type != 'x') return false;

  char to_name = *s++;
  if ((to_name < '1' || to_name > '9') && (to_name <'a' && to_name > 'h')) return false;

  if (*s != '\0') return false;

  int from_column = row_name - 'a';
  int from_row    = col_name - '1';
  int dist;

  move->from = to_position(from_row, from_column);

  if (to_name >= '1' && to_name <= '8') {
    // Vertical move.
    int to_row = to_name - '1';
    dist = to_row - from_row;
    move->to = to_position(to_row, from_column);

  } else {
    // Horizontal move..
    int to_column = to_name - 'a';
    dist = to_column - from_column;
    move->to = to_position(from_row, to_column);
  }

  if (move_type == 'x') {
    if (abs(dist) != 2 && abs(dist) != 3) return false;
    move->capture = move->from + _capture_delta_to_regular_delta(dist) * (move->to - move->from) / dist;
    move->capture_piece = get_piece(board, move->capture);
  } else {
    if (abs(dist) != 1 && abs(dist) != 2) return false;
    move->capture = INV_POSITION;
  }

  return true;
}

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

const char *CLI_TOP_ROW = " abcdefgh\n";
const char *CLI_TOP_ROW_INV = " hgfedcba\n";

const char CLI_WHITE_CELL = '_';
const char CLI_BLACK_CELL = ' ';
const char CLI_WHITE_CELL_CENTER = '.';
const char CLI_BLACK_CELL_CENTER = '.';
const char CLI_WHITE_PAWN = 'P';
const char CLI_WHITE_KNIGHT = 'N';
const char CLI_BLACK_PAWN = 'p';
const char CLI_BLACK_KNIGHT = 'n';
const char CLI_ERROR = 'E';
const char CLI_ISLANDS_EMPTY = '.';
const char CLI_ISLANDS_NO = '+';
const char CLI_ISLANDS_YES = '#';

void print_board(board_t *board, bool player) {
  printf("%s", player ? CLI_TOP_ROW_INV : CLI_TOP_ROW);

  for (int prow = 0; prow < 8; prow++) {
    int row = perspective_row(prow, player);
    printf("%c", row_name(row));

    for (int pcol = 0; pcol < 8; pcol++) {
      int col = perspective_col(pcol, player);

      pos_t position = to_position(row, col);
      piece_t piece = get_piece(board, position);

      char character;
      switch (piece) {
      case EMPTY:
        if (is_center(position)) {
          character = (col + row) % 2 ? CLI_WHITE_CELL_CENTER : CLI_BLACK_CELL_CENTER;
        } else {
          character = (col + row) % 2 ? CLI_WHITE_CELL : CLI_BLACK_CELL;
        }
        break;

      case WHITE_PAWN:
        character = CLI_WHITE_PAWN;
        break;
      case WHITE_KNIGHT:
        character = CLI_WHITE_KNIGHT;
        break;
      case BLACK_PAWN:
        character = CLI_BLACK_PAWN;
        break;
      case BLACK_KNIGHT:
        character = CLI_BLACK_KNIGHT;
        break;
      default:
        character = CLI_ERROR;
        break;
      }

      printf("%c", character);
    }
    printf("\n");
  }
}

void print_islands(board_t* board, state_cache_t* state, bool player) {
  for (int prow=0; prow<8; prow++) {
    int row = perspective_row(prow, player);

    for (int pcol=0; pcol<8; pcol++) {
      int col = perspective_col(pcol, player);
      pos_t pos = to_position(row, col);
      char c;
      if (get_piece(board, pos) == EMPTY) {
        c = CLI_ISLANDS_EMPTY;
      } else {
        c = state->islands[pos] ? CLI_ISLANDS_YES : CLI_ISLANDS_NO;
      }
      printf("%c", c);
    }
    printf("\n");
  }
}

void print_eval(eval_t eval, board_t *board, history_t* history) {
  switch (eval.type) {
  case WHITE_WINS:
    printf("white mates in %lu\n", eval.strength - history->size);
    break;
  case BLACK_WINS:
    printf("black mates in %lu\n", eval.strength - history->size);
    break;
  case CONTINUE:
    printf("continue with advantage %i\n", eval.strength);
    break;
  case NOT_CALCULATED:
    printf("not calculated\n");
    break;
  }
}

void print_moves(move_t* moves, size_t length) {
  printf("{ ");

  if (length) {
    print_move(moves[0]);
  }

  for (int i=1; i<length; i++) {
    printf(" ");
    print_move(moves[i]);
  }

  printf(" }");
}
