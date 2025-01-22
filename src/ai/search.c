#include "ai/search.h"
#include "ai/eval_t.h"
#include "ai/move_ordering.h"
#include "ai/position_evaluation.h"
#include "io/pp.h"
#include "move/generation.h"
#include "move/make_move.h"

#include <stdbool.h>
#include <stdio.h>

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

  if (cache->cancel_search) {
    if (starting_move) {
      pp_f("[canceled search]\n");
    }

    *best_moves_length = 0;
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

  // Order moves for better pruning.
  order_moves(state, cache, moves, moves_length, state->turn);

  eval_t best_evaluation = EVAL_INVALID;
  *best_moves_length = 0;

  // Loop through all of the available moves except the first, and recursively get the next moves.
  for (int i=0; i<moves_length; i++) {
    move_t move = moves[i];
    size_t new_depth = max_depth - 1;
    eval_t evaluation;
    size_t new_moves_length;

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
                             NULL,
                             &new_moves_length,
                             alpha,
                             beta,
                             false);

      // If the shallow search returned a great move, do a full search.
      full_search = (evaluation < best_evaluation) ^ state->turn;
    }

    if (full_search) {
      evaluation = _evaluate(state,
                             history,
                             cache,
                             new_depth,
                             NULL,
                             &new_moves_length,
                             alpha,
                             beta,
                             false);
    }

    undo_last_move(state, history);

    // Print evaluation.
    if (starting_move) {
      io_debug();
      pp_f("debug: for move ");
      pp_move(move);
      pp_f(" -- ");
      pp_eval(evaluation, state->board, history);
      pp_f("\n");
    }

    if (evaluation == EVAL_INVALID) {
      *best_moves_length = 0;
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
