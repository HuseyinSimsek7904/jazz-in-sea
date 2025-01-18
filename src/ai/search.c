#include "board/board_t.h"
#include "board/pos_t.h"
#include "board/piece_t.h"
#include "move/move_t.h"
#include "move/generation.h"
#include "move/make_move.h"
#include "ai/measure_count.h"
#include "ai/search.h"
#include "ai/transposition_table.h"
#include "io/pp.h"

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

static inline int _get_evaluation(board_state_t* state, ai_cache_t* cache) {
  int eval = 0;
  for (int row=0; row<8; row++) {
    for (int col=0; col<8; col++) {
      pos_t pos = to_position(row, col);
      piece_t piece = get_piece(state->board, pos);
      char piece_color = get_piece_color(piece);
      int piece_eval;

      if (piece_color == MOD_EMPTY) continue;

      switch (get_piece_type(piece)) {
      case MOD_PAWN:
        piece_eval = state->islands[pos] ? cache->pawn_island_adv_table[pos] : cache->pawn_adv_table[pos];
        break;
      case MOD_KNIGHT:
        piece_eval = state->islands[pos] ? cache->knight_island_adv_table[pos] : cache->knight_adv_table[pos];
        break;
      default:
        assert(false);
        return 0;
      }

      if (piece_color == MOD_BLACK) piece_eval = -piece_eval;
      eval += piece_eval;
    }
  }
  return eval;
}

// Find the best continuing moves available and their evaluation value.
eval_t
_evaluate(board_state_t* state,
          history_t* history,
          ai_cache_t* cache,
          size_t max_depth,
          move_t* best_moves,
          size_t* best_moves_length,
          eval_t alpha,
          eval_t beta,
          bool starting_move) {

  // Can be used to debug whilst trying to optimise the evaluate function.
#ifdef MEASURE_EVAL_COUNT
  evaluate_count++;
#endif

  // Check for the board state.
  // If the game should not continue, return the evaluation.
  // No need to memorize, as it will take equally as long.
  // No need to add to the transposition table though, as it will take equally as long.
  switch (state->status & 0x30) {
  case 0x10:
#ifdef MEASURE_EVAL_COUNT
    game_end_count++;
#endif
    return 0;
  case 0x20:
#ifdef MEASURE_EVAL_COUNT
    game_end_count++;
#endif
    return EVAL_WHITE_MATES - history->size;
  case 0x30:
#ifdef MEASURE_EVAL_COUNT
    game_end_count++;
#endif
    return EVAL_BLACK_MATES + history->size;
  }

#ifdef MM_OPT_TRANSPOSITION
  // Check if this board was previously calcuated.
  {
    eval_t possible_eval = try_find_tt(cache,
                                       state->hash,
                                       history,
                                       max_depth,
                                       alpha,
                                       beta);

    if (possible_eval != EVAL_INVALID) {
#ifdef MEASURE_EVAL_COUNT
      tt_remember_count++;
#endif
      return possible_eval;
    }
  }
#endif

  // Check if we reached the end of the best_line buffer.
  // If so, just return the evaluation.
  // No need to add to transposition table as finding a depth 0 branch is almost useless.
  if (!max_depth) {
#ifdef MEASURE_EVAL_COUNT
    leaf_count++;
#endif

    return _get_evaluation(state, cache);
  }

  move_t moves[256];
  int moves_length = generate_moves(state, moves);

  // Check for draw by no moves.
  if (!moves_length) {
    return 0;
  }

  // If this move is the first move and if there are only one possible moves, return the only move.
  // No need to search recursively.
  if (starting_move && moves_length == 1) {
#ifdef MEASURE_EVAL_COUNT
    leaf_count++;
#endif
    best_moves[0] = moves[0];
    *best_moves_length = 1;
    return EVAL_INVALID;
  }

  eval_t best_evaluation = EVAL_INVALID;
  *best_moves_length = 0;

  // Loop through all of the available moves except the first, and recursively get the next moves.
  for (int i=0; i<moves_length; i++) {
    // If the move was a capture move, do not decrement the depth.
    move_t move = moves[i];
#ifdef MM_OPT_EXC_DEEPENING
    size_t new_depth = is_capture(move) ? max_depth : max_depth - 1;
#else
    size_t new_depth = max_depth - 1;
#endif
    size_t new_moves_length;

#if defined(TEST_EVAL_STATE) && !defined(NDEBUG)
    board_t _test_old_board = *board;
    state_cache_t _test_old_state = *state;
#endif

    do_move(state, history, move);

    eval_t evaluation = _evaluate(state, history, cache, new_depth, NULL, &new_moves_length, alpha, beta, false);

    undo_last_move(state, history);

#ifdef TEST_EVAL_STATE
    assert(_test_old_state.hash == state->hash && compare(board, (board_t *)&_test_old_board));
    assert(_test_old_state.white_count == state->white_count);
    assert(_test_old_state.white_island_count == state->white_island_count);
    assert(_test_old_state.black_count == state->black_count);
    assert(_test_old_state.black_island_count == state->black_island_count);
    assert(_test_old_state.status == state->status);
#endif

    if (starting_move) {
      io_debug();
      pp_f("debug: ");
      pp_move(move);
      pp_f(" -> ");
      pp_eval(evaluation, state->board, history);
      pp_f("\n");
    }

    // If this move is not the first move, compare this move with the best move.
    if (*best_moves_length) {
      // Compare this move and the old best move.
      if (evaluation == best_evaluation) {
        // If this move is equally as good as the best move, add this move to the list.
        if (starting_move) best_moves[(*best_moves_length)++] = move;
        continue;
      }

      if ((evaluation > best_evaluation) ^ state->turn) {
        // If this move is worse than the found moves, continue.
        continue;
      }
    }

    best_evaluation = evaluation;
    *best_moves_length = 1;
    if (starting_move) *best_moves = move;

    // Update the limit variables alpha and beta.
    if (state->turn) {
      if (evaluation > beta) {
#ifdef MEASURE_EVAL_COUNT
        ab_branch_cut_count++;
#endif

#ifdef MM_OPT_TRANSPOSITION
        // try_add_tt(cache, state->hash, history, max_depth, best_evaluation, LOWER);
#endif
        return best_evaluation;
      }

      if (evaluation > alpha)
        alpha = evaluation;

    } else {
      if (evaluation < alpha) {
#ifdef MEASURE_EVAL_COUNT
        ab_branch_cut_count++;
#endif

#ifdef MM_OPT_TRANSPOSITION
        // try_add_tt(cache, state->hash, history, max_depth, best_evaluation, UPPER);
#endif
        return best_evaluation;
      }

      if (evaluation < beta)
        beta = evaluation;
    }
  }

#ifdef MM_OPT_TRANSPOSITION
  try_add_tt(cache, state->hash, history, max_depth, best_evaluation, EXACT);
#endif

  return best_evaluation;
}

eval_t
evaluate(board_state_t *state,
         history_t *history,
         size_t max_depth,
         move_t *best_moves,
         size_t *best_moves_length) {

  // Reset the measuring variables.
#ifdef MEASURE_EVAL_COUNT
  evaluate_count = 0;
  ab_branch_cut_count = 0;
  game_end_count = 0;
  leaf_count = 0;

#ifdef MM_OPT_TRANSPOSITION
  tt_remember_count = 0;
  tt_saved_count = 0;
  tt_overwritten_count = 0;
  tt_rewritten_count = 0;
#endif

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

  io_debug();
  pp_f("debug: calling _evaluate with depth %u for %s\n", max_depth, state->turn ? "white" : "black");
  pp_board(state->board, false);

  eval_t evaluation = _evaluate(state,
                                history,
                                &cache,
                                max_depth,
                                best_moves,
                                best_moves_length,
                                EVAL_BLACK_MATES,
                                EVAL_WHITE_MATES,
                                true);

  io_debug();
  pp_moves(best_moves, *best_moves_length);
  pp_f(" -> ");
  pp_eval(evaluation, state->board, history);
  pp_f("\n");

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

#ifdef MM_OPT_TRANSPOSITION
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

#endif

  free_cache(&cache);

  return evaluation;
}
