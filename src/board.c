#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "board.h"
#include "position.h"

const char *TOP_ROW;
const char *TOP_ROW_INV;

const char WHITE_CELL = '_';
const char BLACK_CELL = ' ';
const char WHITE_CELL_CENTER = '.';
const char BLACK_CELL_CENTER = '.';
const char WHITE_PAWN = 'P';
const char WHITE_KNIGHT = 'N';
const char BLACK_PAWN = 'p';
const char BLACK_KNIGHT = 'n';
const char ERROR = 'E';

// Get a piece at a position.
// Asserts if position is invalid.
char get_piece(board_t *board, pos_t pos) {
#ifndef NDEBUG
  assert(board->initialized);
#endif

  assert(is_valid_pos(pos));
  return board->board_array[pos];
}

// Set a piece at a position.
// Asserts if position is invalid.
void set_piece(board_t *board, pos_t pos, char piece) {
  assert(is_valid_pos(pos));
  board->board_array[pos] = piece;
}

void print_board(board_t *board, bool reverse) {
  printf(reverse ? " hgfedcba\n" : " abcdefgh\n");
  for (int prow = 0; prow < 8; prow++) {
    int row = perspective_row(prow, reverse);
    printf("%c", '0' + row + 1);

    for (int pcol = 0; pcol < 8; pcol++) {
      int col = perspective_col(pcol, reverse);

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

// Change the turn of the board to the other player, and return the new turn.
bool next_turn(board_t *board) { return board->turn = !board->turn; }

void copy_board(board_t *src, board_t *dest) {
#ifndef NDEBUG
  dest->initialized = src->initialized;
#endif

  // Copy the turn value.
  dest->turn = src->turn;

  // Copy the board contents.
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8; col++) {
      pos_t position = to_position(row, col);
      set_piece(dest, position, get_piece(src, position));
    }
  }
}

// Check if the boards are same.
bool compare(board_t *board1, board_t *board2) {
  // Uninitialized boards cannot be compared.
#ifndef NDEBUG
  assert(board1->initialized);
  assert(board2->initialized);
#endif

  if (board1->turn != board2->turn)
    return false;

  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8; col++) {
      pos_t position = to_position(row, col);

      if (get_piece(board1, position) != get_piece(board2, position))
        return false;
    }
  }

  return true;
}
