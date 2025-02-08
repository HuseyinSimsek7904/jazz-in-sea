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

#ifndef _AI_MEASURE_COUNT_H
#define _AI_MEASURE_COUNT_H

#include <stddef.h>

#ifdef MEASURE_EVAL_COUNT
extern size_t position_evaluation_count;
extern size_t move_generation_count;

extern size_t evaluate_count;
extern size_t game_end_count;
extern size_t leaf_count;
extern size_t ab_branch_cut_count;

extern size_t tt_remember_count;
extern size_t tt_saved_count;
extern size_t tt_overwritten_count;
extern size_t tt_rewritten_count;
#endif

#endif
