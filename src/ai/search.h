#ifndef _AI_SEARCH_H
#define _AI_SEARCH_H

#include "ai/eval_t.h"
#include "ai/transposition_table.h"
#include "ai/cache.h"
#include "board/board_t.h"
#include "board/board_t.h"
#include "board/pos_t.h"
#include "state/board_state_t.h"
#include "state/history.h"
#include "move/move_t.h"

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

eval_t evaluate(board_state_t*, history_t*, size_t, move_t*, size_t*);

#endif
