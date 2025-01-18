#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "board/board_t.h"
#include "board/board_t.h"
#include "board/piece_t.h"
#include "state/status.h"
#include "state/hash_operations.h"
#include "move/move_t.h"
#include "io/pp.h"

// Update the island table using the current piece.
void _generate_islands_pos(board_state_t* state, pos_t position, char color) {
  if (!is_valid_pos(position)) return;
  if (get_piece_color(get_piece(state->board, position)) != color) return;
  if (state->islands[position]) return;

  state->islands[position] = true;

  if (color == MOD_WHITE)
    state->white_island_count++;
  else
    state->black_island_count++;

  for (int i=0; i<4; i++) {
    _generate_islands_pos(state, position + deltas[i], color);
  }
}

// Create the island table.
// This table can later be used to check if a move caused a piece to change an island.
void generate_islands(board_state_t* state) {

  // Count the number of island pieces.
  state->white_island_count = 0;
  state->black_island_count = 0;

  for (int row=0; row<8; row++) {
    for (int col=0; col<8; col++) {
      state->islands[to_position(row, col)] = false;
    }
  }

  // Generates state->islands on the center squares for both colors.
  _generate_islands_pos(state, 0x33, MOD_WHITE);
  _generate_islands_pos(state, 0x34, MOD_WHITE);
  _generate_islands_pos(state, 0x43, MOD_WHITE);
  _generate_islands_pos(state, 0x44, MOD_WHITE);
  _generate_islands_pos(state, 0x33, MOD_BLACK);
  _generate_islands_pos(state, 0x34, MOD_BLACK);
  _generate_islands_pos(state, 0x43, MOD_BLACK);
  _generate_islands_pos(state, 0x44, MOD_BLACK);

  // Quick check for if the number of island pieces are greater than the total number of pieces.
  assert(state->white_island_count <= state->white_count);
  assert(state->black_island_count <= state->black_count);
}

// Generate the square hashes table.
static inline void _generate_hash_tables(board_state_t *state) {
  state->turn_hash = (long) rand() << 32 | rand();

  for (int row=0; row<8; row++) {
    for (int col=0; col<8; col++) {
      for (int piece=0; piece<4; piece++) {
        state->hash_tables[piece][to_position(row, col)] = (long) rand() << 32 | rand();
      }
    }
  }
}

// Generate a state cache from only the information given on the board.
void generate_state_cache(board_state_t* state, history_t* history) {
  // Count the pieces on the board.
  state->white_count = 0;
  state->black_count = 0;

  for (int row=0; row<8; row++) {
    for (int col=0; col<8; col++) {
      char piece_color = get_piece_color(get_piece(state->board, to_position(row, col)));

      if (piece_color == MOD_WHITE)
        state->white_count++;
      else if (piece_color == MOD_BLACK)
        state->black_count++;
    }
  }

  // Generate the hash tables.
  _generate_hash_tables(state);

  // Generate the hash value for the board.
  generate_full_hash(state);

  // Update the islands and the game status.
  generate_islands(state);
  generate_board_status(state, history);
}
