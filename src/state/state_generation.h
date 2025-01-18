#ifndef _STATE_STATE_GENERATION_H
#define _STATE_STATE_GENERATION_H

#include "state/board_state_t.h"
#include "state/history.h"
#include "board/hash_t.h"

#include <stdbool.h>

void generate_islands(board_state_t* state);
void generate_state_cache(board_state_t* state, history_t* history);

#endif
