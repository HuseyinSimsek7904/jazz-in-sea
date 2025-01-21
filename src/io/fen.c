#include "board/board_t.h"
#include "board/piece_t.h"
#include "board/pos_t.h"
#include "state/state_generation.h"
#include "io/fen.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define FEN_WHITE_PAWN 'P'
#define FEN_WHITE_KNIGHT 'N'
#define FEN_BLACK_PAWN 'p'
#define FEN_BLACK_KNIGHT 'n'

static inline piece_t _char_to_piece(char c) {
  switch (c) {
  case FEN_WHITE_PAWN:
    return WHITE_PAWN;
  case FEN_WHITE_KNIGHT:
    return WHITE_KNIGHT;
  case FEN_BLACK_PAWN:
    return BLACK_PAWN;
  case FEN_BLACK_KNIGHT:
    return BLACK_KNIGHT;
  default:
    assert(false);
    return -1;
  }
}

static inline char _piece_to_char(piece_t piece) {
  switch (piece) {
  case WHITE_PAWN:
    return FEN_WHITE_PAWN;
  case WHITE_KNIGHT:
    return FEN_WHITE_KNIGHT;
  case BLACK_PAWN:
    return FEN_BLACK_PAWN;
  case BLACK_KNIGHT:
    return FEN_BLACK_KNIGHT;
  default:
    assert(false);
    return -1;
  }
}

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
bool load_fen_string(const char* fen, board_state_t* state, history_t* history) {
  int row = 0, col = 0;

  for (; *fen != ' '; fen++) {
    switch (*fen) {
    case '/':
      if (col != 8 || row >= 8) return false;

      // Skip to next row.
      row++;
      col = 0;
      break;

    case '1' ... '8':;
      // Add spaces.
      // Technically, 8/62/8/... is not a valid sentence as there should not be digits right next to each other.
      // However checking this seems unnecessary.
      int spaces = *fen - '0';
      if (col + spaces > 8) return false;
      while (spaces--) set_piece(state->board, to_position(row, col++), EMPTY);
      break;

    case FEN_WHITE_PAWN:
    case FEN_WHITE_KNIGHT:
    case FEN_BLACK_PAWN:
    case FEN_BLACK_KNIGHT:
      if (row > 7 || col > 7) return false;

      // Add the corresponding piece.
      set_piece(state->board, to_position(row, col++), _char_to_piece(*fen));
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
    state->turn = true;
    break;
  case 'b':
    state->turn = false;
    break;
  default:
    return false;
  }

  // Check if we reached the end of the string.
  if (*fen != '\0') return false;

#if defined (TEST_BOARD_INIT) && ! NDEBUG
  board->initialized = true;
#endif

  // Reset the history.
  history->size = 0;

  // Update the board state.
  generate_state_cache(state, history);

  return true;
}

// Save a board to FEN string.
// fen must be an array of chars at least 75 bytes long. Just use 256 bytes.
char* get_fen_string(char* fen, board_state_t* state) {
  for (int row=0; row<8; row++) {
    for (int col=0; col<8; col++) {
      piece_t piece = get_piece(state->board, to_position(row, col));

      if (piece == EMPTY) {
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
  *fen++ = _player_to_char(state->turn);

  *fen = '\0';
  return fen;
}


#define MAX_BUFFER 0x1000

// Load a board from a FEN file.
bool load_fen_from_path(const char *path, board_state_t* state, history_t *history) {
  char buffer[MAX_BUFFER];

  FILE* file = fopen(path, "r");
  if (!file)
    return false;

  if (!fgets(buffer, MAX_BUFFER, file))
    return false;

  fclose(file);

  return load_fen_string(buffer, state, history);
}

// Save a board to a FEN file.
bool save_fen_to_path(const char* path, board_state_t* state) {
  char buffer[256];

  get_fen_string(buffer, state);

  FILE* file = fopen(path, "w");
  if (!file)
    return false;

  fprintf(file, "%s\n", buffer);

  fclose(file);

  return true;
}