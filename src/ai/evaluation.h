#ifndef _AI_EVALUATION_H
#define _AI_EVALUATION_H

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
#include <time.h>

eval_t evaluate(board_state_t*, history_t*, size_t, struct timespec, move_t*, size_t*);

#endif
