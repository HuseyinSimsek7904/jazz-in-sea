#ifndef _AI_TRANSPOSITION_TABLE_H
#define _AI_TRANSPOSITION_TABLE_H

#ifdef MM_OPT_TRANSPOSITION

#include "ai/cache.h"
#include "ai/eval_t.h"
#include "state/history.h"
#include "board/hash_t.h"

unsigned int get_tt_saved_count();
unsigned int get_tt_overwritten_count();
unsigned int get_tt_rewritten_count();

tt_entry_t* get_entry_tt(ai_cache_t *, hash_t);

void try_add_tt(ai_cache_t*, hash_t, size_t, size_t, eval_t, node_type_t);
eval_t try_find_tt(ai_cache_t*, hash_t, size_t, size_t, eval_t, eval_t);
#endif

#endif
