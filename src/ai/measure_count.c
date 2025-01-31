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

#include "ai/measure_count.h"

#ifdef MEASURE_EVAL_COUNT
size_t evaluate_count = 0;
size_t game_end_count = 0;
size_t leaf_count = 0;
size_t ab_branch_cut_count = 0;

size_t tt_remember_count = 0;
size_t tt_saved_count = 0;
size_t tt_overwritten_count = 0;
size_t tt_rewritten_count = 0;
#endif
