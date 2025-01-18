#ifndef _STATE_HISTORY_H
#define _STATE_HISTORY_H

#include "board/hash_t.h"
#include "move/move_t.h"

#include <stdbool.h>

#define HISTORY_DEPTH 0x1000

typedef struct {
  hash_t hash;
  move_t move;
} history_item_t;

typedef struct {
  history_item_t history[HISTORY_DEPTH];
  size_t size;
} history_t;

bool check_for_repetition(history_t*, hash_t, size_t);

#endif
