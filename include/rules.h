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
} state_t;

size_t generate_moves(board_t*, move_t[256]);
state_t get_board_state(board_t*);
const char *board_state_text(state_t);

void _generate_islands_pos(board_t*, bool[256], pos_t, bool);
void generate_islands(board_t *, bool[256]);

bool islands_should_be_updated(move_t, bool[256]);

#endif
