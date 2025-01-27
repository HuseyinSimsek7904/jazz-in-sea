/*
This file is part of JazzInSea.

JazzInSea is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

JazzInSea is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with JazzInSea. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _AI_EVAL_T_H
#define _AI_EVAL_T_H

#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>

typedef int eval_t;

#define EVAL_INVALID     (eval_t) INT_MIN
#define EVAL_WHITE_MATES (eval_t) INT_MAX
#define EVAL_BLACK_MATES (eval_t) INT_MIN + 1

static inline bool is_mate(eval_t eval) { return abs(eval) > 1000000; }
static inline int mate_depth(eval_t eval) { return EVAL_WHITE_MATES - abs(eval); }

#endif
