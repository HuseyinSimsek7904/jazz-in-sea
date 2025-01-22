#define _GNU_SOURCE
#include "ai/cache.h"
#include "ai/eval_t.h"
#include "ai/iterative_deepening.h"
#include "ai/search.h"
#include "ai/measure_count.h"
#include "ai/transposition_table.h"
#include "board/board_t.h"
#include "board/pos_t.h"
#include "board/piece_t.h"
#include "move/move_t.h"
#include "move/generation.h"
#include "move/make_move.h"
#include "io/pp.h"
#include "state/board_state_t.h"
#include "state/history.h"

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

// As x increases, distance from center in x decreases.
// Same for y.
const int TOPLEFT_PAWN_ADV_TABLE[4][4] = {
  {-520, -470, -420, -320},
  {-470, -420, -370, -270},
  {-420, -370, -320, -220},
  {-320, -270, -220,  -20},
};

const int TOPLEFT_PAWN_ISLAND_ADV_TABLE[4][4] = {
  {-150, -100,  -50,    0},
  {-100,  -50,    0,   50},
  { -50,    0,   50,  100},
  {   0,   50,  100,  120},
};

const int TOPLEFT_KNIGHT_ADV_TABLE[4][4] = {
  { 400,  450,  500,  600},
  { 450,  500,  550,  660},
  { 500,  550,  670,  650},
  { 600,  670,  650,  670}
};

const int TOPLEFT_KNIGHT_ISLAND_ADV_TABLE[4][4] = {
  { 400,  450,  500,  600},
  { 450,  500,  550,  660},
  { 500,  550,  720,  650},
  { 600,  670,  650,  700}
};

eval_t
evaluate(board_state_t *state,
         history_t *history,
         size_t max_depth,
         struct timespec max_time,
         move_t *best_moves,
         size_t *best_moves_length) {

  // Reset the measuring variables.
#ifdef MEASURE_EVAL_COUNT
  evaluate_count = 0;
  ab_branch_cut_count = 0;
  game_end_count = 0;
  leaf_count = 0;

  tt_remember_count = 0;
  tt_saved_count = 0;
  tt_overwritten_count = 0;
  tt_rewritten_count = 0;
#endif

  ai_cache_t cache;
  setup_cache(&cache,
              TOPLEFT_PAWN_ADV_TABLE,
              TOPLEFT_PAWN_ISLAND_ADV_TABLE,
              TOPLEFT_KNIGHT_ADV_TABLE,
              TOPLEFT_KNIGHT_ISLAND_ADV_TABLE);


#ifdef MEASURE_EVAL_TIME
  clock_t start = clock();
#endif

  eval_t evaluation;

  pthread_t thread;
  _id_routine_args_t args = {
    .state = state,
    .cache = &cache,
    .history = history,
    .best_moves = best_moves,
    .best_moves_length = best_moves_length,
    .evaluation = &evaluation,
    .max_depth = max_depth,
  };
  cache.cancel_search = false;
  pthread_create(&thread, NULL, _id_routine, &args);

  struct timespec abs_end;
  clock_gettime(CLOCK_REALTIME, &abs_end);
  abs_end.tv_sec += max_time.tv_sec;
  abs_end.tv_nsec += max_time.tv_nsec;

  abs_end.tv_sec += abs_end.tv_nsec / 1000000000;
  abs_end.tv_nsec %= 1000000000;

  int result = pthread_timedjoin_np(thread, NULL, &abs_end);
  if (result == ETIMEDOUT) {
    cache.cancel_search = true;
    pthread_join(thread, NULL);
  }

#ifdef MEASURE_EVAL_TIME
  pp_f("measure: took %dms\n", (clock() - start) / (CLOCKS_PER_SEC / 1000));
#endif

#ifdef MEASURE_EVAL_COUNT
  pp_f("measure: called _evaluate %d times.\n",
       evaluate_count);
  pp_f("measure: cut %d branches.\n",
       ab_branch_cut_count);
  pp_f("measure: found %d (%d %%) different game ends.\n",
       game_end_count,
       game_end_count * 100 / evaluate_count);
  pp_f("measure: found total %d (%d %%) leaves.\n",
       leaf_count,
       leaf_count * 100 / evaluate_count);

  pp_f("measure: in total, used %d (%d %%) transposition tables entries.\n",
       tt_saved_count,
       tt_saved_count * 100 / AI_HASHMAP_SIZE);
  if (tt_saved_count != 0) {
    pp_f("measure: remembered %d (%d %% per call, %d %% per entry) times.\n",
         tt_remember_count,
         tt_remember_count * 100 / evaluate_count,
         tt_remember_count * 100 / tt_saved_count);
    pp_f("measure: overwritten the same board %u (%u %%) times.\n",
         tt_overwritten_count,
         tt_overwritten_count * 100 / tt_saved_count);
    pp_f("measure: rewritten a different board %u (%u %%) times.\n",
         tt_rewritten_count,
         tt_rewritten_count * 100 / tt_saved_count);
  }
#endif

  free_cache(&cache);

  return evaluation;
}
