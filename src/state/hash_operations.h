#ifndef _STATE_HASH_OPERATIONS_H
#define _STATE_HASH_OPERATIONS_H

#include "state/board_state_t.h"
#include "board/pos_t.h"

void update_hash_for_piece(board_state_t* state, piece_t piece, pos_t pos);
void generate_full_hash(board_state_t *state);

#endif
