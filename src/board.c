#include <stdbool.h>
#include <stdio.h>

#include "board.h"

char get_piece(board_t* board, int row, int column) {
  return board->board_array[row][column];
}

void set_piece(board_t* board, int row, int column, char piece) {
  board->board_array[row][column] = piece;
}

bool load_fen(const char* fen, board_t* board) {
  int row = 0;
  int col = 0;

  for (; *fen != ' '; fen++) {
    switch (*fen) {
      // Skip to next row.
    case '/':
      if (col != 8 || row >= 8)
        return false;

      row++;
      col = 0;
      break;

      // Add spaces.
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':;
      int spaces = *fen - '0';
      if (col + spaces > 8)
        return false;

      while (spaces--)
        set_piece(board, row, col++, ' ');

      break;

      // Add the corresponding piece.
    case 'p':
    case 'n':
    case 'P':
    case 'N':
      if (row > 7 || col > 7)
        return false;

      board->board_array[row][col++] = *fen;
      break;
    }
  }
  if (row != 7 || col != 8) {
    return false;
  }

  return true;
}

int perspective_row(int row, bool reverse) {
  // if reverse is true : perspective of the white player
  // if reverse is false: perspective of the black player
  return reverse ? row : 7 - row;
}

int perspective_col(int col, bool reverse) {
  // if reverse is true : perspective of the white player
  // if reverse is false: perspective of the black player
  return reverse ? 7 - col : col;
}

void print_board(board_t* board, bool reverse) {
  printf(reverse ? " hgfedcba\n" : " abcdefgh\n");
  for (int row=0; row<8; row++) {
    int board_row = perspective_row(row, reverse);
    printf("%c", '0' + board_row + 1);

    for (int col=0; col<8; col++) {
      int board_col = perspective_col(col, reverse);

      char piece = get_piece(board, board_row, board_col);

      if (piece == ' ') {
        printf("%c", (board_col + board_row) % 2 ? ' ' : '_');
      } else {
        printf("%c", piece);
      }
    }
    printf("\n");
  }
}
