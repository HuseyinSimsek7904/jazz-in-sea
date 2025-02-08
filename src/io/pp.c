/*
This file is part of JazzInSea.

JazzInSea is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

JazzInSea is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
JazzInSea. If not, see <https://www.gnu.org/licenses/>.
*/

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "ai/search.h"
#include "board/board_t.h"
#include "board/piece_t.h"
#include "board/pos_t.h"
#include "io/pp.h"
#include "move/move_t.h"

// Try to convert a char to a piece.
piece_t char_to_piece(char c) {
  switch (c) {
  case 'P':
    return WHITE_PAWN;
  case 'N':
    return WHITE_KNIGHT;
  case 'p':
    return BLACK_PAWN;
  case 'n':
    return BLACK_KNIGHT;
  default:
    return 0;
  }
}

// Try to convert a string to a position.
bool string_to_position(const char *s, pos_t *pos) {
  char col_name = *s++;
  if (col_name < 'a' || col_name > 'h')
    return false;

  char row_name = *s++;
  if (row_name < '1' || row_name > '8')
    return false;

  if (*s != '\0')
    return false;

  *pos = to_position(row_name - '1', col_name - 'a');
  return true;
}

// Given the delta multiplier of a move's capture position, return the move's
// regular delta multiplier.
int _capture_delta_to_regular_delta(int dist) {
  switch (dist) {
  case -3:
    return -2;
  case -2:
    return -1;
  case 2:
    return 1;
  case 3:
    return 2;
  default:
    assert(false);
    return 0;
  }
}

// Try to convert a string to a move.
bool string_to_move(const char *s, board_t board, move_t *move) {
  char row_name = *s++;
  if (row_name < 'a' || row_name > 'h')
    return false;

  char col_name = *s++;
  if (col_name < '1' || col_name > '8')
    return false;

  char move_type = *s++;
  if (move_type != '>' && move_type != 'x')
    return false;

  char to_name = *s++;
  if ((to_name < '1' || to_name > '9') && (to_name < 'a' && to_name > 'h'))
    return false;

  if (*s != '\0')
    return false;

  int from_column = row_name - 'a';
  int from_row = col_name - '1';
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
    if (abs(dist) != 2 && abs(dist) != 3)
      return false;
    move->capture = move->from + _capture_delta_to_regular_delta(dist) *
                                     (move->to - move->from) / dist;
    move->capture_piece = board[move->capture];
  } else {
    if (abs(dist) != 1 && abs(dist) != 2)
      return false;
    move->capture = POSITION_INV;
  }

  return true;
}

// Get the column name from a column number.
// aka 0->a, 1->b ...  7->h
char col_name(int col) { return 'a' + col; }

// Get the row name from a row number.
// aka 0->1, 1->2, ... 7->8
char row_name(int row) { return '1' + row; }

// Print a position.
// The notation is: <column-name><row-number>
void fprint_position(FILE *file, pos_t pos) {
  fprintf(file, "%c%c", col_name(to_col(pos)), row_name(to_row(pos)));
}

// Print a move.
// The notation for regular moves is: <from_position> > <to_row_or_col_name>
// The notation for capture moves is: <from_position> x <to_row_or_col_name>
// If the piece moved horizontally, <to_row_or_col_name> is the to column name.
// If the piece moved vertically, <to_row_or_col_name> is the to row name.
void fprint_move(FILE *file, move_t move) {
  // Check if the move is valid.
  if (!is_valid_move(move)) {
    fprintf(file, "<invalid move>");
    return;
  }

  fprint_position(file, move.from);
  fprintf(file, "%c", is_valid_pos(move.capture) ? 'x' : '>');

  if (to_col(move.from) == to_col(move.to)) {
    // Column numbers are equal, so the piece moved vertically.
    fprintf(file, "%c", row_name(to_row(move.to)));

  } else {
    // This should be true according to the normal Cez rules.
    // Because pieces move only horizontally or vertically.
    assert(to_row(move.from) == to_row(move.to));

    fprintf(file, "%c", col_name(to_col(move.to)));
  }
}

const char *CLI_TOP_ROW = " abcdefgh\n";

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

void fprint_board(FILE *file, board_t board) {
  fprintf(file, "%s", CLI_TOP_ROW);

  for (int row = 8; row-- > 0;) {
    fprintf(file, "%c", row_name(row));

    for (int col = 0; col < 8; col++) {
      pos_t position = to_position(row, col);
      piece_t piece = board[position];

      char character;
      switch (piece) {
      case EMPTY:
        if (is_center(position)) {
          character =
              (col + row) % 2 ? CLI_WHITE_CELL_CENTER : CLI_BLACK_CELL_CENTER;
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

      fprintf(file, "%c", character);
    }
    fprintf(file, "\n");
  }
}

void fprint_islands(FILE *file, board_state_t *state) {
  fprintf(file, "%s", CLI_TOP_ROW);

  for (int row = 8; row-- > 0;) {
    fprintf(file, "%c", row_name(row));

    for (int col = 0; col < 8; col++) {
      pos_t position = to_position(row, col);
      char c;
      if (state->board[position] == EMPTY) {
        c = CLI_ISLANDS_EMPTY;
      } else {
        c = (state->islands_bb & (1ull << position)) ? CLI_ISLANDS_YES : CLI_ISLANDS_NO;
      }
      fprintf(file, "%c", c);
    }
    fprintf(file, "\n");
  }
}

void fprint_eval(FILE *file, eval_t eval, history_t *history) {
  if (eval == EVAL_INVALID) {
    fprintf(file, "NA");
  } else if (is_mate(eval)) {
    if (eval > 0) {
      fprintf(file, "WM#%lu", mate_depth(eval) - history->size);
    } else {
      fprintf(file, "BM#%lu", mate_depth(eval) - history->size);
    }
  } else if (eval > 0) {
    fprintf(file, "+%i", eval);
  } else {
    fprintf(file, "%i", eval);
  }
}

void fprint_moves(FILE *file, move_t *moves) {
  fprintf(file, "{ ");

  for (int i = 0; is_valid_move(moves[i]); i++) {
    fprint_move(file, moves[i]);
    fprintf(file, " ");
  }

  fprintf(file, "}");
}

int generate_argv(char *buffer, char *arg_buffer, char **argv,
                  const size_t arg_buffer_size, const size_t argv_size) {
  int argc = 0;
  char *buffer_ptr = buffer;
  char *arg_buffer_ptr = arg_buffer;

  while (true) {
    // Ignore the whitespaces.
    char c;
    do {
      c = *buffer_ptr++;
    } while (is_whitespace(c));

    // If reached the end of the string, finish parsing.
    if (c == '\0')
      break;

    char quote = '\0';
    if (c == '\'' || c == '"') {
      quote = c;
      c = *buffer_ptr++;
    }

    // Copy the string from the buffer to arg buffer.
    if (argc > argv_size) {
      io_error();
      pp_f("error: argv overflow\n");
      return -1;
    }
    argv[argc++] = arg_buffer_ptr;

    while (quote ? c != quote : !is_whitespace(c)) {
      // Check for buffer overflow and move the character.
      if (arg_buffer_ptr > arg_buffer + arg_buffer_size) {
        io_error();
        pp_f("error: arg buffer overflow\n");
        return -1;
      }
      *arg_buffer_ptr++ = c;

      if (c == '\0') {
        if (quote) {
          io_error();
          pp_f("error: unterminated quote\n");
          return -1;
        }

        // After all of the buffers are filled, add a null ptr to the
        // arg_ptr_buffer.
        if (argc > argv_size) {
          io_error();
          pp_f("error: argv overflow\n");
          return -1;
        }

        argv[argc] = 0;
        return argc;
      }

      c = *buffer_ptr++;
    }

    // Check for buffer overflow and move the character.
    if (arg_buffer_ptr > arg_buffer + arg_buffer_size) {
      io_error();
      pp_f("error: arg buffer overflow\n");
      return -1;
    }
    *arg_buffer_ptr++ = '\0';
  }

  // After all of the buffers are filled, add a null ptr to the arg_ptr_buffer.
  if (argc > argv_size) {
    io_error();
    pp_f("error: argv overflow\n");
    return -1;
  }

  argv[argc] = 0;
  return argc;
}
