#ifndef _RULES_H
#define _RULES_H

#include <stddef.h>

#include "board.h"
#include "move.h"

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
  // The current board status.
  status_t status;
} state_cache_t;

size_t generate_moves(board_t*, move_t[256]);
const char *board_status_text(status_t);
void _generate_islands_pos(board_t*, bool[256], pos_t, bool);
void generate_islands(board_t *, bool[256]);

bool islands_should_be_updated(move_t, bool[256]);

void generate_state_cache(board_t *, state_cache_t *);

void do_move(board_t*, state_cache_t*, move_t);
void undo_move(board_t*, state_cache_t*, move_t);

#endif
