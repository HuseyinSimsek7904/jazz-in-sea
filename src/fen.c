#include "fen.h"
#include "board.h"
#include "position.h"

// Load (and initialize) a board from FEN string.
bool load_fen(const char* fen, board_t* board) {
  int row = 0, col = 0;

  for (; *fen != ' '; fen++) {
    switch (*fen) {
    case '/':
      if (col != 8 || row >= 8) return false;

      // Skip to next row.
      row++;
      col = 0;
      break;

    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':;
      // Add spaces.
      // Technically, 8/62/8/... is not a valid sentence as there should not be digits right next to each other.
      // However checking this seems unnecessary.
      int spaces = *fen - '0';
      if (col + spaces > 8) return false;
      while (spaces--) set_piece(board, to_position(row, col++), ' ');
      break;

    case 'p':
    case 'n':
    case 'P':
    case 'N':
      if (row > 7 || col > 7) return false;

      // Add the corresponding piece.
      set_piece(board, to_position(row, col++), *fen);
      break;

    default:
      return false;
    }
  }

  // Check if we reached the end of the board.
  if (row != 7 || col != 8) return false;

  // Get the current player information.
  fen++;
  switch (*fen++) {
  case 'w':
    board->turn = true;
    break;
  case 'b':
    board->turn = false;
    break;
  default:
    return false;
  }

  // Check if we reached the end of the string.
  if (*fen != '\0') return false;

  #ifndef NDEBUG
  board->initialized = true;
  #endif

  return true;
}
