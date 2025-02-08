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

#include "ai/cache.h"
#include "board/pos_t.h"

void setup_cache(ai_cache_t *cache, const int topleft_pawn[4][4],
                 const int topleft_knight[4][4],
                 const int topleft_pawn_centered[4][4],
                 const int topleft_knight_centered[4][4],
                 const int topleft_pawn_island[4][4],
                 const int topleft_knight_island[4][4]) {

  cache->cancel_search = false;

  // Load the advantage tables.
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8; col++) {
      int topleft_row = row;
      int topleft_col = col;

      if (row >= 4)
        topleft_row = 7 - row;
      if (col >= 4)
        topleft_col = 7 - col;

      pos_t pos = to_position(row, col);

      cache->pawn_adv_table[pos] = topleft_pawn[topleft_row][topleft_col];
      cache->knight_adv_table[pos] = topleft_knight[topleft_row][topleft_col];

      cache->pawn_centered_adv_table[pos] =
          topleft_pawn[topleft_row][topleft_col];
      cache->knight_centered_adv_table[pos] =
          topleft_knight[topleft_row][topleft_col];

      cache->pawn_island_adv_table[pos] =
          topleft_pawn_island[topleft_row][topleft_col];
      cache->knight_island_adv_table[pos] =
          topleft_knight_island[topleft_row][topleft_col];
    }
  }

  // For now, load constant values.
  cache->centered_adv = 200;

  cache->est_evaluation_pos = 1;
  cache->est_evaluation_old = 10;
  cache->est_evaluation_killer = 50;

  cache->late_move_reduction = 8;
  cache->late_move_min_depth = 3;
  cache->exchange_deepening = 2;

  cache->tt_size = 0x200000;

  cache->transposition_table = malloc(sizeof(tt_entry_t) * cache->tt_size);

  // Depth < 0 on a memorized item indicates not set.
  for (size_t i = 0; i < cache->tt_size; i++) {
    cache->transposition_table[i] = (tt_entry_t){.eval = EVAL_INVALID};
  }
}

void free_cache(ai_cache_t *cache) { free(cache->transposition_table); }
