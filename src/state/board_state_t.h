#ifndef _STATE_BOARD_STATE_T_H
#define _STATE_BOARD_STATE_T_H

#include "board/piece_t.h"
#include "board/hash_t.h"
#include "board/status_t.h"

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
  hash_t hash_tables[4][256];
  hash_t turn_hash;

  // The current hash value.
  hash_t hash;
} board_state_t;

#endif
