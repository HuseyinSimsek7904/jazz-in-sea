#include "ai/search.h"
#include "ai/eval_t.h"
#include "ai/move_ordering.h"
#include "ai/position_evaluation.h"
#include "io/pp.h"
#include "move/generation.h"
#include "move/make_move.h"
#include "move/move_t.h"

#include <stdbool.h>
#include <stdio.h>

// Find the best continuing moves available and their evaluation value.
eval_t
_evaluate(board_state_t* state,
          history_t* history,
          ai_cache_t* cache,
          size_t max_depth,
          eval_t alpha,
          eval_t beta,
          move_t* killer_moves) {

  if (cache->cancel_search) {
    return EVAL_INVALID;
  }

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

  // Check if this board was previously calcuated.
  {
    eval_t possible_eval = try_find_tt(cache,
                                       state->hash,
                                       history->size,
                                       max_depth > 1 ? max_depth : 1,
                                       alpha,
                                       beta);

    if (possible_eval != EVAL_INVALID) {
#ifdef MEASURE_EVAL_COUNT
      tt_remember_count++;
#endif
      return possible_eval;
    }
  }

  // Check if we reached the end of the best_line buffer.
  // If so, just return the evaluation.
  // No need to add to transposition table as finding a depth 0 branch is almost useless.
  if (!max_depth) {
#ifdef MEASURE_EVAL_COUNT
    leaf_count++;
#endif

    return get_board_evaluation(state, cache);
  }

  move_t moves[256];
  int moves_length = generate_moves(state, moves);

  // Check for draw by no moves.
  if (!moves_length) {
    return 0;
  }

  // Order moves for better pruning.
  order_moves(state, cache, moves, moves_length, state->turn, killer_moves);

  eval_t best_evaluation = state->turn ? EVAL_BLACK_MATES : EVAL_WHITE_MATES;
  move_t new_killer_moves[256] = { INV_MOVE };

  // Loop through all of the available moves except the first, and recursively get the next moves.
  for (int i=0; i<moves_length; i++) {
    move_t move = moves[i];
    size_t new_depth = max_depth - 1;
    eval_t evaluation;

    // Since capture moves are important and should be searched better,
    // we extend the search depth for these moves.
    if (is_capture(move))
      new_depth += cache->exchange_deepening;

    // Test if the board state changes after making and unmaking moves.
#if defined(TEST_EVAL_STATE) && !defined(NDEBUG)
    board_t _test_old_board = *board;
    state_cache_t _test_old_state = *state;
#endif

    do_move(state, history, move);

    // Since we already know that after move ordering, late moves are probably bad.
    // Because of that, do a shallower search on them. If they are too good, do a full search.
    bool full_search = true;
    if (i >= cache->late_move_reduction && new_depth >= cache->late_move_min_depth) {
      evaluation = _evaluate(state,
                             history,
                             cache,
                             new_depth - 1,
                             alpha,
                             beta,
                             new_killer_moves);

      // If the shallow search returned a great move, do a full search.
      full_search = (evaluation < best_evaluation) ^ state->turn;
    }

    if (full_search) {
      evaluation = _evaluate(state,
                             history,
                             cache,
                             new_depth,
                             alpha,
                             beta,
                             new_killer_moves);
    }

    undo_last_move(state, history);

    if (evaluation == EVAL_INVALID) {
      return EVAL_INVALID;
    }

#ifdef TEST_EVAL_STATE
    assert(_test_old_state.hash == state->hash && compare(board, (board_t *)&_test_old_board));
    assert(_test_old_state.white_count == state->white_count);
    assert(_test_old_state.white_island_count == state->white_island_count);
    assert(_test_old_state.black_count == state->black_count);
    assert(_test_old_state.black_island_count == state->black_island_count);
    assert(_test_old_state.status == state->status);
#endif

    // If this move is not better than the found moves, continue.
    if (state->turn ? evaluation <= best_evaluation : evaluation >= best_evaluation) {
      continue;
    }

    best_evaluation = evaluation;

    // Update the limit variables alpha and beta.
    if (state->turn) {
      if (evaluation > beta) {
#ifdef MEASURE_EVAL_COUNT
        ab_branch_cut_count++;
#endif

        // Add this move to killer moves.
        int i=0;
        while (is_valid_move(killer_moves[i])) i++;
        killer_moves[i] = move;
        killer_moves[i + 1] = INV_MOVE;

        // try_add_tt(cache, state->hash, history->size, max_depth, best_evaluation, LOWER);
        return best_evaluation;
      }

      if (evaluation > alpha)
        alpha = evaluation;

    } else {
      if (evaluation < alpha) {
#ifdef MEASURE_EVAL_COUNT
        ab_branch_cut_count++;
#endif

        // Add this move to killer moves.
        int i=0;
        while (is_valid_move(killer_moves[i])) i++;
        killer_moves[i] = move;
        killer_moves[i + 1] = INV_MOVE;

        // try_add_tt(cache, state->hash, history->size, max_depth, best_evaluation, UPPER);
        return best_evaluation;
      }

      if (evaluation < beta)
        beta = evaluation;
    }
  }

  try_add_tt(cache, state->hash, history->size, max_depth, best_evaluation, EXACT);

  return best_evaluation;
}
