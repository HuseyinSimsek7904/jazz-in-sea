#include <assert.h>
#include <stddef.h>

#include "rules.h"
#include "board.h"
#include "move.h"
#include "position.h"
#include "piece.h"

// Direction units.
// These macros are defined using the perspective of the white player.
#define UP    -0x10
#define DOWN  +0x10
#define LEFT  -0x01
#define RIGHT +0x01

const int deltas[] = { UP, DOWN, LEFT, RIGHT };

// Generate all possible moves on the board, and place them on the moves array.
// Returns the number of moves.
size_t generate_moves(board_t* board, move_t moves[256]) {
  bool capture_available = false;
  int length = 0;

  for (int row=0; row<8; row++) {
    for (int col=0; col<8; col++) {
      unsigned int position = to_position(row, col);

      char piece = get_piece(board, position);

      // Check if the color of the piece is the color of the player.
      if (!is_piece_color(piece, board->turn)) continue;

      // Calculate the delta length of regular moves.
      int delta_mul = 1;
      if (is_piece_knight(piece)) {
        delta_mul = 2;
      }

      // Loop through all directions.
      for (size_t i=0; i<4; i++) {
        // Calculate the first position and check if it is valid.
        int first_pos = position + deltas[i] * delta_mul;
        if (!is_valid_pos(first_pos)) continue;

        char first_piece = get_piece(board, first_pos);

        if (first_piece == ' ') {
          // The destination position is empty.
          // If there are any available captures, no need to try to find a regular move.
          if (capture_available) continue;

          // Set the move object.
          moves[length++] = (move_t) {
            .from = position,
            .to = first_pos,
            .capture = INV_POSITION,
          };

        } else if (is_piece_color(first_piece, !board->turn)) {
          // The destination position has a piece of opposite color and check if it is valid.
          int second_pos = first_pos + deltas[i];
          if (!is_valid_pos(second_pos)) continue;

          // Check if the destination position is empty.
          if (get_piece(board, second_pos) != ' ') continue;

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
