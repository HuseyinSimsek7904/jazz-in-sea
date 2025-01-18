#include "state/hash_operations.h"
#include "board/board_t.h"

// Update the hash value after placing or removing a piece to a position.
void update_hash_for_piece(board_state_t* state, piece_t piece, pos_t pos) {
  state->hash ^= state->hash_tables[piece - WHITE_PAWN][pos];
}

// Generate the hash value for a board.
void generate_full_hash(board_state_t* state) {
  state->hash = state->turn ? state->turn_hash : 0;
  for (int row=0; row<8; row++) {
    for (int col=0; col<8; col++) {
      pos_t pos = to_position(row, col);
      piece_t piece = get_piece(state->board, pos);

      if (piece != EMPTY)
        update_hash_for_piece(state, piece, pos);
    }
  }
}
