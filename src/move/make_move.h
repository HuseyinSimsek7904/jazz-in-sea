#ifndef _MOVE_MAKE_MOVE_H
#define _MOVE_MAKE_MOVE_H

#include "state/board_state_t.h"
#include "state/history.h"

bool place_piece(board_state_t*, history_t*, pos_t, piece_t);
bool remove_piece(board_state_t*, history_t*, pos_t);

bool do_move(board_state_t*, history_t*, move_t);
bool undo_last_move(board_state_t*, history_t*);

#endif
