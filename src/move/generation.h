#ifndef _MOVE_GENERATION_H
#define _MOVE_GENERATION_H

#include "move/move_t.h"
#include "state/board_state_t.h"

#include <stddef.h>

void generate_moves(board_state_t*, move_t[256]);

#endif
