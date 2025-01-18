#include "move/generation.h"

#include <stddef.h>

// Generate all possible moves on the board, and place them on the moves array.
// Returns the number of moves.
size_t generate_moves(board_state_t* state, move_t moves[256]) {
  char piece_color = MOD_BLACK;
  char opposite_color = MOD_WHITE;

  if (state->turn) {
    piece_color = MOD_WHITE;
    opposite_color = MOD_BLACK;
  }

  bool capture_available = false;
  int length = 0;

  for (int row=0; row<8; row++) {
    for (int col=0; col<8; col++) {
      unsigned int position = to_position(row, col);
      piece_t piece = get_piece(state->board, position);

      // Check if the color of the piece is the color of the player.
      if (get_piece_color(piece) != piece_color) continue;

      bool is_knight = get_piece_type(piece) == MOD_KNIGHT;

      // Loop through all directions.
      for (size_t i=0; i<4; i++) {
        int delta = deltas[(i + get_quadrant(position)) % 4];

        // Calculate the first position and check if it is valid.
        int first_pos = position + delta;
        if (is_knight) first_pos += delta;

        if (!is_valid_pos(first_pos)) continue;

        piece_t first_piece = get_piece(state->board, first_pos);

        if (first_piece == MOD_EMPTY) {
          // The destination position is empty.
          // If there are any available captures, no need to try to find a regular move.
          if (capture_available) continue;

          // Set the move object.
          moves[length++] = (move_t) {
            .from = position,
            .to = first_pos,
            .capture = INV_POSITION,
          };

        } else if (get_piece_color(first_piece) == opposite_color) {
          // The destination position has a piece of opposite color and check if it is valid.
          int second_pos = first_pos + delta;
          if (!is_valid_pos(second_pos)) continue;

          // Check if the destination position is empty.
          if (get_piece(state->board, second_pos) != EMPTY) continue;

          // If we have not found any captures yet, clear all previous moves.
          if (!capture_available) {
            capture_available = true;
            length = 0;
          }

          // Set the move object.
          moves[length++] = (move_t) {
            .from = position,
            .to = second_pos,
            .capture = first_pos,
            .capture_piece = first_piece
          };
        }
      }
    }
  }

  return length;
}
