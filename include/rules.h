#ifndef _RULES_H
#define _RULES_H

#include <stddef.h>
#include <sys/types.h>

#include "board.h"
#include "move.h"
#include "piece.h"
#include "position.h"

#define HISTORY_DEPTH 0x1000

typedef u_int64_t hash_t;

typedef struct {
  hash_t hash;
  move_t move;
} history_item_t;

typedef struct {
  history_item_t history[HISTORY_DEPTH];
  size_t size;
} history_t;

typedef enum {
  NORMAL                 = 0x00,
  DRAW_BY_REPETITION     = 0x10, // When a position is repeated three times.
  DRAW_BY_BOTH_ISLANDS   = 0x11, // When both players has created islands.
  DRAW_BY_NO_MOVES       = 0x12, // When there are no moves available for the current player.
  DRAW_BY_INSUF_MAT      = 0x13, // When neither of the players have any pieces. (Technically not possible)
  WHITE_WON_BY_ISLANDS   = 0x20, // When only white has created islands.
  WHITE_WON_BY_INSUF_MAT = 0x21, // When black loses all material.
  BLACK_WON_BY_ISLANDS   = 0x30, // When only black has created islands.
  BLACK_WON_BY_INSUF_MAT = 0x31, // When white loses all material.
} status_t;

typedef struct {
  // The current board.
  piece_t board[256];

  // The current color to move.
  bool turn;

  // The current board status.
  status_t status;

  // Number of pieces of both players.
  unsigned int white_count;
  unsigned int black_count;

  // Number of pieces that are in islands for both players.
  unsigned int white_island_count;
  unsigned int black_island_count;

  // Islands table.
  bool islands[256];

  // Square hashes table used to generate a hash value for boards.
  hash_t square_hash[4][256];

  // The current hash value.
  hash_t hash;
} board_state_t;

size_t generate_moves(board_state_t*, move_t[256]);
const char *board_status_text(status_t);

void generate_state_cache(board_state_t *, history_t *);

bool place_piece(board_state_t*, history_t*, pos_t, piece_t);
bool remove_piece(board_state_t*, history_t*, pos_t);

void do_move(board_state_t*, history_t*, move_t);
void undo_last_move(board_state_t*, history_t*);

#endif
