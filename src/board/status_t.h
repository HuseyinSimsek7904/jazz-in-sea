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

#ifndef _BOARD_STATUS_T_H
#define _BOARD_STATUS_T_H

typedef enum {
  NORMAL = 0x00,
  DRAW_BY_REPETITION = 0x10,   // When a position is repeated three times.
  DRAW_BY_BOTH_ISLANDS = 0x11, // When both players has created islands.
  DRAW_BY_NO_MOVES =
      0x12, // When there are no moves available for the current player.
  DRAW_BY_INSUF_MAT = 0x13,      // When neither of the players have any pieces.
                                 // (Technically not possible)
  WHITE_WON_BY_ISLANDS = 0x20,   // When only white has created islands.
  WHITE_WON_BY_INSUF_MAT = 0x21, // When black loses all material.
  BLACK_WON_BY_ISLANDS = 0x30,   // When only black has created islands.
  BLACK_WON_BY_INSUF_MAT = 0x31, // When white loses all material.
} status_t;

static inline const char *board_status_text(status_t status) {
  const char *text = "<unknown state>";

  switch (status) {
  case NORMAL:
    text = "continue";
    break;
  case DRAW_BY_REPETITION:
    text = "draw by repetition";
    break;
  case DRAW_BY_BOTH_ISLANDS:
    text = "draw by both players have islands";
    break;
  case DRAW_BY_NO_MOVES:
    text = "draw by no moves available";
    break;
  case DRAW_BY_INSUF_MAT:
    text = "draw by both sides have insufficient material";
    break;
  case WHITE_WON_BY_ISLANDS:
    text = "white won by islands";
    break;
  case WHITE_WON_BY_INSUF_MAT:
    text = "white won as opponent has insufficient material";
    break;
  case BLACK_WON_BY_ISLANDS:
    text = "black won by islands";
    break;
  case BLACK_WON_BY_INSUF_MAT:
    text = "black won as opponent has insufficient material";
    break;
  }

  return text;
}

#endif
