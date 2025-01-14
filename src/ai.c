#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "ai.h"
#include "board.h"
#include "io.h"
#include "move.h"
#include "position.h"
#include "piece.h"
#include "rules.h"

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

// Compare eval1 and eval2 by whether they are favorable for the white player.
// <0 => eval2 is more favorable than eval1
// =0 => neither is more favorable
// >0 => eval1 is more favorable than eval2
// NOT_CALCULATED evals can not be compared.
int compare_eval(eval_t eval1, eval_t eval2) {
  switch (eval1.type) {
  case WHITE_WINS:
    // Unless eval2 is WHITE_WINS, always eval1.
    // Otherwise compare the depths.
    return eval2.type != WHITE_WINS ? 1 : eval2.strength - eval1.strength;

  case BLACK_WINS:
    // Unless eval2 is BLACK_WINS, always eval2.
    // Otherwise compare the depths.
    return eval2.type != BLACK_WINS ? -1 : eval1.strength - eval2.strength;

  case CONTINUE:
    switch (eval2.type) {
      // If eval2 is WHITE_WINS, always eval2.
    case WHITE_WINS: return -1;
      // If eval2 is BLACK_WINS, always eval1.
    case BLACK_WINS: return 1;
      // If eval2 is CONTINUE, check for the evaluation difference.
    case CONTINUE: return eval1.strength - eval2.strength;

    case NOT_CALCULATED:
      assert(false);
      break;
    }

  case NOT_CALCULATED:
    assert(false);
    break;
  }

  return 0;
}

// Compare eval1 and eval2 by whether they are favorable for the colored player.
// <0 => eval2 is more favorable than eval1
// =0 => neither is more favorable
// >0 => eval1 is more favorable than eval2
// NOT_CALCULATED evals can not be compared.
int compare_eval_by(eval_t eval1, eval_t eval2, bool color) {
  int cmp = compare_eval(eval1, eval2);

  if (!color)
    cmp = -cmp;

  return cmp;
}

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

#ifdef MEASURE_EVAL_COUNT
size_t evaluate_count = 0;
size_t game_end_count = 0;
size_t leaf_count = 0;
size_t ab_branch_cut_count = 0;

unsigned int get_evaluate_count() { return evaluate_count; }
unsigned int get_game_end_count() { return game_end_count; }
unsigned int get_leaf_count() { return leaf_count; }
unsigned int get_ab_branch_cut_count() { return ab_branch_cut_count; }

#ifdef MM_OPT_MEMOIZATION
size_t remember_count = 0;
size_t saved_count = 0;
unsigned int get_remember_count() { return remember_count; }
unsigned int get_saved_count() { return saved_count; }
#endif

#endif

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
  switch (state->status & 0x30) {
  case 0x10:
#ifdef MEASURE_EVAL_COUNT
    game_end_count++;
#endif
    return (eval_t) { .type=CONTINUE, .strength=0 };
  case 0x20:
#ifdef MEASURE_EVAL_COUNT
    game_end_count++;
#endif
    return (eval_t) { .type=WHITE_WINS, .strength=history->size };
  case 0x30:
#ifdef MEASURE_EVAL_COUNT
    game_end_count++;
#endif
    return (eval_t) { .type=BLACK_WINS, .strength=history->size };
  }

#ifdef MM_OPT_MEMOIZATION
  // Check if this board was previously calcuated.
  {
    eval_t possible_eval = try_remember(cache,
                                        state->hash,
                                        history,
                                        max_depth,
                                        alpha,
                                        beta);

    if (possible_eval.type != NOT_CALCULATED) {
#ifdef MEASURE_EVAL_COUNT
      remember_count++;
#endif
      return possible_eval;
    }
  }
#endif

  // Check if we reached the end of the best_line buffer.
  // If so, just return the evaluation.
  // No need to memorize as finding a depth 0 branch is almost useless.
  if (!max_depth) {
#ifdef MEASURE_EVAL_COUNT
    leaf_count++;
#endif

    return (eval_t) { .type=CONTINUE, .strength=_get_evaluation(state, cache) };
  }

  move_t moves[256];
  int moves_length = generate_moves(state, moves);

  // Check for draw by no moves.
  if (!moves_length) {
    return (eval_t) { .type=CONTINUE, .strength=0 };
  }

  // If this move is the first move and if there are only one possible moves, return the only move.
  // No need to search recursively or memorize.
  if (starting_move && moves_length == 1) {
#ifdef MEASURE_EVAL_COUNT
    leaf_count++;
#endif
    best_moves[0] = moves[0];
    *best_moves_length = 1;
    return (eval_t) { .type=NOT_CALCULATED };
  }

  eval_t best_evaluation;
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

    // If this move is not the first move, compare this move with the best move.
    if (*best_moves_length) {
      // Compare this move and the old best move.
      int cmp = compare_eval_by(evaluation, best_evaluation, state->turn);

      if (cmp < 0) {
        // If this move is worse than the found moves, continue.
        continue;

      } else if (cmp == 0) {
        // If this move is equally as good as the best move, add this move to the list.
        if (starting_move) best_moves[(*best_moves_length)++] = move;
        continue;
      }
    }

    best_evaluation = evaluation;
    *best_moves_length = 1;
    if (starting_move) *best_moves = move;

    // Update the limit variables alpha and beta.
    if (state->turn) {
      if (compare_eval(evaluation, beta) > 0) {
#ifdef MEASURE_EVAL_COUNT
        ab_branch_cut_count++;
#endif
        memorize(cache, state->hash, history, max_depth, best_evaluation, LOWER);
        return best_evaluation;
      }

      if (compare_eval(evaluation, alpha) > 0)
        alpha = evaluation;

    } else {
      if (compare_eval(evaluation, alpha) < 0) {
#ifdef MEASURE_EVAL_COUNT
        ab_branch_cut_count++;
#endif
        memorize(cache, state->hash, history, max_depth, best_evaluation, UPPER);
        return best_evaluation;
      }

      if (compare_eval(evaluation, beta) < 0)
        beta = evaluation;
    }
  }

#ifdef MM_OPT_MEMOIZATION
  memorize(cache, state->hash, history, max_depth, best_evaluation, EXACT);
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

#ifdef MM_OPT_MEMOIZATION
  remember_count = 0;
  saved_count = 0;
#endif

#endif

  ai_cache_t cache;
  setup_cache(&cache,
              TOPLEFT_PAWN_ADV_TABLE,
              TOPLEFT_PAWN_ISLAND_ADV_TABLE,
              TOPLEFT_KNIGHT_ADV_TABLE,
              TOPLEFT_KNIGHT_ISLAND_ADV_TABLE);

  eval_t evaluation = _evaluate(state,
                                history,
                                &cache,
                                max_depth,
                                best_moves,
                                best_moves_length,
                                (eval_t) { .type=BLACK_WINS, .strength=0 },  // best possible evaluation for black
                                (eval_t) { .type=WHITE_WINS, .strength=0 },  // best possible evaluation for white
                                true);

  free_cache(&cache);

  return evaluation;
}

void setup_cache(ai_cache_t* cache,
                 const int topleft_pawn[4][4],
                 const int topleft_pawn_island[4][4],
                 const int topleft_knight[4][4],
                 const int topleft_knight_island[4][4]) {

  // Load the advantage tables.
  for (int row=0; row<8; row++) {
    for (int col=0; col<8; col++) {
      int topleft_row = row;
      int topleft_col = col;

      if (row >= 4) topleft_row = 7 - row;
      if (col >= 4) topleft_col = 7 - col;

      pos_t pos = to_position(row, col);

      cache->pawn_adv_table[pos] = topleft_pawn[topleft_row][topleft_col];
      cache->knight_adv_table[pos] = topleft_knight[topleft_row][topleft_col];
      cache->pawn_island_adv_table[pos] = topleft_pawn_island[topleft_row][topleft_col];
      cache->knight_island_adv_table[pos] = topleft_knight_island[topleft_row][topleft_col];
    }
  }

#ifdef MM_OPT_MEMOIZATION
  cache->memorized = malloc(sizeof(memorized_t) * AI_HASHMAP_SIZE);

  for (size_t i=0; i<AI_HASHMAP_SIZE; i++) {
    (*cache->memorized)[i].depth = 0;
  }
#endif
}

void free_cache(ai_cache_t* cache) {
#ifdef MM_OPT_MEMOIZATION
  free(cache->memorized);
#endif
}

#ifdef MM_OPT_MEMOIZATION
// Add the board to the memorized boards.
void
memorize(ai_cache_t* cache,
         hash_t hash,
         history_t* history,
         size_t depth,
         eval_t eval,
         node_type_t node_type) {

  // If the eval is an absolute evaluation, convert the depth relative.
  if (eval.type == WHITE_WINS || eval.type == BLACK_WINS) {
    eval.strength -= history->size;
    depth = LONG_MAX;
  }

  memorized_t* memorized = &(*cache->memorized)[hash % AI_HASHMAP_SIZE];

  if (depth <= memorized->depth) return;

  *memorized = (memorized_t) {
    .hash = hash,
    .depth = depth,
    .eval = eval,
    .node_type = node_type,
  };

#ifdef MEASURE_EVAL_COUNT
  saved_count++;
#endif
}

// Get if the board was saved for memoization before.
eval_t
try_remember(ai_cache_t* cache,
             hash_t hash,
             history_t* history,
             size_t depth,
             eval_t alpha,
             eval_t beta) {

  memorized_t* memorized = &(*cache->memorized)[hash % AI_HASHMAP_SIZE];

  if (memorized->hash != hash || memorized->depth < depth) return (eval_t) { .type=NOT_CALCULATED };

  // If the eval is an absolute evaluation, convert the depth absolute as well.
  if (memorized->eval.type == WHITE_WINS || memorized->eval.type == BLACK_WINS) {
    memorized->eval.strength += history->size;
  }

  switch (memorized->node_type) {
  case EXACT:
    break;
  case LOWER:
    if (compare_eval(memorized->eval, alpha) > 0) return (eval_t) { .type=NOT_CALCULATED };
  case UPPER:
    if (compare_eval(memorized->eval, beta) < 0) return (eval_t) { .type=NOT_CALCULATED };
  }

  return memorized->eval;
}
#endif
