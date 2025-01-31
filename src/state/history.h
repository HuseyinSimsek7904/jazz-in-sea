/*
This file is part of JazzInSea.

JazzInSea is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

JazzInSea is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
JazzInSea. If not, see <https://www.gnu.org/licenses/>.
*/

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

bool check_for_repetition(history_t *, hash_t, size_t);

#endif
