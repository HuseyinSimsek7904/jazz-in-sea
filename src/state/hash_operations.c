#include "state/hash_operations.h"
#include "board/board_t.h"
#include "board/hash_t.h"
#include "board/piece_t.h"
#include "board/pos_t.h"
#include "state/board_state_t.h"

// Generate the hash value for a board.
void generate_full_hash(board_state_t* state) {
  state->hash = state->turn ? state->turn_hash : 0;
  for (int row=0; row<8; row++) {
    for (int col=0; col<8; col++) {
      pos_t pos = to_position(row, col);
      piece_t piece = get_piece(state->board, pos);

      if (piece != EMPTY)
        state->hash ^= get_hash_for_piece(state, piece, pos);
    }
  }
}
