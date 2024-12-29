#include "fen.h"
#include "board.h"
#include "piece.h"
#include "position.h"
#include "rules.h"
#include <assert.h>

#define FEN_WHITE_PAWN 'P'
#define FEN_WHITE_KNIGHT 'N'
#define FEN_BLACK_PAWN 'p'
#define FEN_BLACK_KNIGHT 'n'

// Right now, we use the FEN representations of piece on the board array.
// However, when this is changed in the future, this function should be updated.
static inline char _char_to_piece(char c) { return c; }
static inline char _piece_to_char(char piece) { return piece; }

static inline char _player_to_char(bool player) { return player ? 'w' : 'b'; }
static inline bool _char_to_player(char c) {
  switch (c) {
  case 'w': return true;
  case 'b': return false;
  default:
    assert(false);
    return false;
  }
}

// Load and initialize a board and its state cache from FEN string.
bool load_fen(const char* fen, state_cache_t* state, board_t* board) {
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

    case FEN_WHITE_PAWN:
    case FEN_WHITE_KNIGHT:
    case FEN_BLACK_PAWN:
    case FEN_BLACK_KNIGHT:
      if (row > 7 || col > 7) return false;

      // Add the corresponding piece.
      set_piece(board, to_position(row, col++), _char_to_piece(*fen));
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

  // Reset the move count.
  board->move_count = 0;

  // Update the board state.
  generate_state_cache(board, state);

  return true;
}

// Save a board to FEN string.
// fen must be an array of chars at least 75 bytes long. Just use 256 bytes.
char* save_fen(char* fen, board_t* board) {
  for (int row=0; row<8; row++) {
    for (int col=0; col<8; col++) {
      char piece = get_piece(board, to_position(row, col));

      if (piece == ' ') {
        if (*(fen - 1) >= '1' && *(fen - 1) <= '8') {
          (*(fen - 1))++;
        } else {
          *fen++ = '1';
        }

      } else {
        *fen++ = _piece_to_char(piece);
      }
    }

    if (row != 7)
      *fen++ = '/';
  }

  *fen++ = ' ';
  *fen++ = _player_to_char(board->turn);

  *fen = '\0';
  return fen;
}
