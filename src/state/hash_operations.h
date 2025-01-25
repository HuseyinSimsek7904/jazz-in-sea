#ifndef _STATE_HASH_OPERATIONS_H
#define _STATE_HASH_OPERATIONS_H

#include "board/piece_t.h"
#include "move/move_t.h"
#include "state/board_state_t.h"
#include "board/pos_t.h"

// Return the new hash value to be xored with the current hash after a piece is placed or removed from a position.
static inline hash_t get_hash_for_piece(board_state_t* state, piece_t piece, pos_t pos) {
  return state->hash_tables[piece - WHITE_PAWN][pos];
}

// Return the new hash value to be xored with the current hash after a piece is moved or "unmoved".
static inline hash_t get_hash_for_move(board_state_t* state, piece_t piece, move_t move) {
  return
    get_hash_for_piece(state, piece, move.from) ^
    get_hash_for_piece(state, piece, move.to) ^
    (is_capture(move) ? get_hash_for_piece(state, move.capture_piece, move.capture) : 0);
}

void generate_full_hash(board_state_t *state);

#endif
