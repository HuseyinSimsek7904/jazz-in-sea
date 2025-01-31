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

#ifndef _AI_TRANSPOSITION_TABLE_H
#define _AI_TRANSPOSITION_TABLE_H

#include "ai/cache.h"
#include "ai/eval_t.h"
#include "board/hash_t.h"
#include "state/history.h"

unsigned int get_tt_saved_count();
unsigned int get_tt_overwritten_count();
unsigned int get_tt_rewritten_count();

tt_entry_t *get_entry_tt(ai_cache_t *, hash_t);

void try_add_tt(ai_cache_t *, hash_t, size_t, size_t, eval_t, node_type_t);
eval_t try_find_tt(ai_cache_t *, hash_t, size_t, size_t, eval_t, eval_t);

#endif
