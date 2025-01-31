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

#ifndef _AI_CACHE_H
#define _AI_CACHE_H

#include "ai/eval_t.h"
#include "board/hash_t.h"

#define AI_HASHMAP_SIZE 0x20000

typedef enum { EXACT, LOWER, UPPER } node_type_t;

typedef struct {
  hash_t hash;
  size_t depth;
  eval_t eval;
  node_type_t node_type;
} tt_entry_t;

typedef struct {
  bool cancel_search;

  // Advantage tables used if there are no centered piece for a color.
  int pawn_adv_table[256];
  int knight_adv_table[256];

  // Advantage tables used if there is at least one centered piece for a color.
  // Used for pieces not inside islands.
  int pawn_centered_adv_table[256];
  int knight_centered_adv_table[256];

  // Used for pieces inside islands.
  int pawn_island_adv_table[256];
  int knight_island_adv_table[256];

  // Added if there is at least one centered piece.
  int centered_adv;

  int est_evaluation_pos;
  int est_evaluation_old;
  int est_evaluation_killer;

  int late_move_reduction;
  int late_move_min_depth;
  int exchange_deepening;

  tt_entry_t (*transposition_table)[AI_HASHMAP_SIZE];
} ai_cache_t;

void setup_cache(ai_cache_t *cache, const int[4][4], const int[4][4],
                 const int[4][4], const int[4][4], const int[4][4],
                 const int[4][4]);

void free_cache(ai_cache_t *);

#endif
