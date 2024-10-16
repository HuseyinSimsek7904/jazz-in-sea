#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

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

// Gets the number of pieces on the board.
void count_pieces(board_t* board, int* white, int* black) {
  for (int row=0; row<8; row++) {
    for (int col=0; col<8; col++) {
      char piece = get_piece(board, to_position(row, col));

      if (is_piece_white(piece)) {
        (*white)++;
      } else if (is_piece_black(piece)) {
        (*black)++;
      }
    }
  }
}

// Counts the cells in the island that contain this position.
unsigned int get_island_size(board_t* board, bool color, bool visited[256], unsigned int position) {
  // Check if the position is already visited.
  if (visited[position]) return 0;

  // Ignore the cells that do not contain pieces of the color we are interested in.
  if (!is_piece_color(get_piece(board, position), color)) return 0;

  visited[position] = true;

  // Add all of the neighbor cells' island size (we start with 1 to add the current cell to the count).
  unsigned int total = 1;
  for (int i=0; i<4; i++) {
    unsigned char new_position = position + deltas[i];

    if (is_valid_pos(new_position))
      total += get_island_size(board, color, visited, position + deltas[i]);
  }
  return total;
}

// As can be seen very clearly, this function is written very poorly.
// This is because this function does not need to be fast right now, we will be
// improving all of the functions as we progress.
// Get the board state.
state_t get_board_state(board_t* board) {
  // Count all of the pieces.
  int white_piece_count = 0, black_piece_count = 0;
  count_pieces(board, &white_piece_count, &black_piece_count);

  if (!white_piece_count)
    return black_piece_count ? BLACK_WON_BY_INSUF_MAT : DRAW_BY_INSUF_MAT;

  if (!black_piece_count)
    return WHITE_WON_BY_INSUF_MAT;

  bool visited[256];

  // Reset all visited entries.
  for (int i=0; i<256; i++) visited[i] = false;

  // Check all of the center positions.
  white_piece_count -=
    get_island_size(board, true, visited, 0x33) +
    get_island_size(board, true, visited, 0x34) +
    get_island_size(board, true, visited, 0x43) +
    get_island_size(board, true, visited, 0x44);

  black_piece_count -=
    get_island_size(board, false, visited, 0x33) +
    get_island_size(board, false, visited, 0x34) +
    get_island_size(board, false, visited, 0x43) +
    get_island_size(board, false, visited, 0x44);

  if (!white_piece_count)
    return black_piece_count ? WHITE_WON_BY_ISLANDS : DRAW_BY_BOTH_ISLANDS;

  if (!black_piece_count)
    return BLACK_WON_BY_ISLANDS;

  return NORMAL;
}

const char* board_state_text(state_t state) {
  const char* text = "<unknown state>";

  switch (state) {
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
