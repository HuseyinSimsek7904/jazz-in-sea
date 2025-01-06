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

const int PAWN_BASE   = -20;
const int KNIGHT_BASE = 700;

// As x increases, distance from center in x decreases.
// Same for y.
const int TOPLEFT_PAWN_ADV_TABLE[4][4] = {
  {-500, -450, -400, -300},
  {-450, -400, -350, -250},
  {-400, -350, -300, -200},
  {-300, -250, -200, -0  },
};

const int TOPLEFT_KNIGHT_ADV_TABLE[4][4] = {
  {-300, -250, -200, -100},
  {-250, -200, -150, -60 },
  {-200, -150, -30 , -50 },
  {-100,  -60, -50 , -0  }
};

int PAWN_ADV_TABLE[256];
int KNIGHT_ADV_TABLE[256];

// Returns the copy of eval from the opponent's POV.
// WHITE_WINS => BLACK_WINS
// BLACK_WINS => WHITE_WINS
// DRAW => DRAW
// CONTINUE(x) => CONTINUE(-x)
eval_t _switch_eval_turn(eval_t eval) {
  switch (eval.type) {
  case WHITE_WINS:
    return (eval_t) { .type=BLACK_WINS, .strength=eval.strength };
  case BLACK_WINS:
    return (eval_t) { .type=WHITE_WINS, .strength=eval.strength };
  case CONTINUE:
    return (eval_t) { .type=CONTINUE, .strength=-eval.strength };

  case DRAW:
  case NOT_CALCULATED:
    return eval;
  }
  return (eval_t) {};
}

// Compare eval1 and eval2 by whether they are favorable for the player.
// <0 => eval2 is more favorable than eval1
// =0 => neither is more favorable
// >0 => eval1 is more favorable than eval2
// NOT_CALCULATED evals can not be compared.
int compare_favor(eval_t eval1, eval_t eval2, bool turn) {
  // Convert the eval structs to whites POV to simplify the function.
  // This way, for all player these can be used to mean the same thing.
  // WHITE_WINS => WINS
  // BLACK_WINS => LOSES
  if (!turn) {
    eval1 = _switch_eval_turn(eval1);
    eval2 = _switch_eval_turn(eval2);
  }

  switch (eval1.type) {
  case WHITE_WINS:
    // Unless eval2 is WHITE_WINS, always eval1.
    // Otherwise compare the depths.
    return eval2.type != WHITE_WINS ? 1 : eval2.strength - eval1.strength;

  case BLACK_WINS:
    // Unless eval2 is BLACK_WINS, always eval2.
    // Otherwise compare the depths.
    return eval2.type != BLACK_WINS ? -1 : eval2.strength - eval1.strength;

  case DRAW:
    switch (eval2.type) {
      // If eval2 is WHITE_WINS, always eval2.
    case WHITE_WINS: return -1;
      // If eval2 is BLACK_WINS, always eval1.
    case BLACK_WINS: return 1;
      // If eval2 is DRAW, check for moves.
      // This comparison is not actually necessary, we are just trying to make the AI choose the simpler lines.
    case DRAW: return eval2.strength - eval1.strength;
      // If eval2 is CONTINUE, check for the evaluation.
    case CONTINUE: return eval2.strength;

    case NOT_CALCULATED:
      assert(false);
      break;
    }

  case CONTINUE:
    switch (eval2.type) {
      // If eval2 is WHITE_WINS, always eval2.
    case WHITE_WINS: return -1;
      // If eval2 is BLACK_WINS, always eval1.
    case BLACK_WINS: return 1;
      // If eval2 is DRAW, check for the evaluation.
    case DRAW: return eval1.strength;
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

static inline int get_delta_eval(board_t* board, move_t move) {
  int delta_evaluation = 0;

  // Add the advantage of the piece.
  // It is a "good" thing that we take the opponent's piece as these pieces have a base value.
  // Losing a piece will make you lose that base value.
  if (is_valid_pos(move.capture)) {
    switch (get_piece_type(move.capture_piece)) {
    case MOD_PAWN:
      delta_evaluation += PAWN_BASE;
      break;
    case MOD_KNIGHT:
      delta_evaluation += KNIGHT_BASE;
      break;
    default:
      assert(false);
    }
  }

  // Add the advantage difference of the from and to positions.
  switch (get_piece_type(get_piece(board, move.from))) {
  case MOD_PAWN:
    delta_evaluation += PAWN_ADV_TABLE[move.to] - PAWN_ADV_TABLE[move.from];
    break;
  case MOD_KNIGHT:
    delta_evaluation += KNIGHT_ADV_TABLE[move.to] - KNIGHT_ADV_TABLE[move.from];
    break;
  default:
    assert(false);
  }

  return board->turn ? delta_evaluation : -delta_evaluation;
}

#ifdef MEASURE_EVAL_COUNT
size_t evaluate_count = 0;
size_t game_end_count = 0;
size_t leaf_count = 0;

unsigned int get_evaluate_count() { return evaluate_count; }
unsigned int get_game_end_count() { return game_end_count; }
unsigned int get_leaf_count() { return leaf_count; }

#ifdef MM_OPT_MEMOIZATION
size_t remember_count = 0;
unsigned int get_remember_count() { return remember_count; }
#endif

#endif

// Find the best continuing moves available and their evaluation value.
size_t
_evaluate(board_t* board,
          state_cache_t* state,
          ai_cache_t* cache,
          size_t max_depth,
          move_t* best_moves,
          eval_t* evaluation,
#ifdef MM_OPT_AB_PRUNING
          eval_t alpha,
          eval_t beta,
#endif
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
    *evaluation = (eval_t) { .type=DRAW,       .strength=board->move_count };
    return 0;
  case 0x20:
#ifdef MEASURE_EVAL_COUNT
    game_end_count++;
#endif
    *evaluation = (eval_t) { .type=WHITE_WINS, .strength=board->move_count };
    return 0;
  case 0x30:
#ifdef MEASURE_EVAL_COUNT
    game_end_count++;
#endif
    *evaluation = (eval_t) { .type=BLACK_WINS, .strength=board->move_count };
    return 0;
  }

#ifdef MM_OPT_MEMOIZATION
  // Check if this board was previously calcuated.
  {
    eval_t possible_eval;
    move_t possible_move;

    if (try_remember(cache, state->hash, board, max_depth, &possible_eval, &possible_move)) {
      remember_count++;
      *best_moves = possible_move;
      *evaluation = possible_eval;
      return 1;
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
    *evaluation = (eval_t) { .type=CONTINUE, .strength=0 };
    return 0;
  }

  move_t moves[256];
  int moves_length = generate_moves(board, moves);

  // There must be at least 1 moves, otherwise we should not pass the state check step.
  assert(moves_length);

  // If this move is the first move and if there are only one possible moves, return the only move.
  // No need to search recursively or memorize.
  if (starting_move && moves_length == 1) {
#ifdef MEASURE_EVAL_COUNT
    leaf_count++;
#endif
    *evaluation = (eval_t) { .type=NOT_CALCULATED };
    *best_moves = moves[0];
    return 1;
  }

  size_t found_moves = 0;

#if defined (TEST_EVAL_HASH) && ! defined (NDEBUG)
  const hash_t old_hash = state->hash;
#endif

  // Loop through all of the available moves except the first, and recursively get the next moves.
  for (int i=0; i<moves_length; i++) {
    // If the move was a capture move, do not decrement the depth.
    move_t move = moves[i];
    eval_t new_evaluation;
    size_t new_depth = max_depth - 1;
    move_t new_moves[256];

    do_move(board, state, move);
    _evaluate(board, state, cache, new_depth, new_moves, &new_evaluation,
#ifdef MM_OPT_AB_PRUNING
              alpha, beta,
#endif
              false);
    undo_move(board, state, move);

    // If the evaluation type was CONTINUE, then add the move delta evaluation.
    if (new_evaluation.type == CONTINUE)
      new_evaluation.strength += get_delta_eval(board, move);

    // If this move is not the first move, compare this move with the best move.
    if (found_moves) {
      // Compare this move and the old best move.
      int cmp = compare_favor(new_evaluation, *evaluation, board->turn);

      if (cmp < 0) {
        // If this move is worse than the found moves, continue.
        continue;

      } else if (cmp == 0) {
        // If this move is equally as good as the best move, add this move to the list.
        best_moves[found_moves++] = move;
        continue;
      }
    }

    *evaluation = new_evaluation;
    found_moves = 1;
    *best_moves = move;

#ifdef MM_OPT_AB_PRUNING
    // If found a move better than beta or alpha, break.
    if (compare_favor(new_evaluation, board->turn ? beta : alpha, board->turn) > 0) {
      break;
    }

    // Update the limit variables alpha and beta.
    if (board->turn) {
      if (compare_favor(new_evaluation, alpha, board->turn) > 0)
        alpha = new_evaluation;
    } else {
      if (compare_favor(new_evaluation, beta, board->turn) > 0)
        beta = new_evaluation;
    }
#endif
  }

#ifdef TEST_EVAL_HASH
  assert(old_hash == state->hash);
#endif

#ifdef MM_OPT_MEMOIZATION
  memorize(cache, state->hash, board, (evaluation->type == WHITE_WINS || evaluation->type == BLACK_WINS) ? LONG_MAX : max_depth, *evaluation, best_moves[rand() % found_moves]);
#endif

  return found_moves;
}

size_t evaluate(board_t* board, state_cache_t* state, size_t max_depth, move_t* moves, eval_t* evaluation) {
#ifdef MEASURE_EVAL_COUNT
  evaluate_count = 0;
  game_end_count = 0;
  leaf_count = 0;

#ifdef MM_OPT_MEMOIZATION
  remember_count = 0;
#endif

#endif

  ai_cache_t cache;
  setup_cache(&cache);

  size_t length = _evaluate(board,
                            state,
                            &cache,
                            max_depth,
                            moves,
                            evaluation,
#ifdef MM_OPT_AB_PRUNING
                            (eval_t) { .type=BLACK_WINS, .strength=0 },  // best possible evaluation for black
                            (eval_t) { .type=WHITE_WINS, .strength=0 },  // best possible evaluation for white
#endif
                            true);

  free_cache(&cache);

  return length;
}

void setup_cache(ai_cache_t* cache) {
#ifdef MM_OPT_MEMOIZATION
  for (size_t i=0; i<AI_HASHMAP_SIZE; i++) {
    ai_cache_node_t* node = cache->memorized[i] = malloc(sizeof(ai_cache_node_t));
    node->next = NULL;
    node->size = 0;
  }
#endif
}

void free_cache(ai_cache_t* cache) {
#ifdef MM_OPT_MEMOIZATION
  for (size_t i=0; i<AI_HASHMAP_SIZE; i++) {
    ai_cache_node_t* node = cache->memorized[i];
    while (node != NULL) {
      ai_cache_node_t* next = node->next;
      free(node);
      node = next;
    }
  }
#endif
}

#ifdef MM_OPT_MEMOIZATION
// Add the board to the memorized boards.
void memorize(ai_cache_t* cache, hash_t hash, board_t* board, size_t depth, eval_t eval, move_t move) {
  ai_cache_node_t* node = cache->memorized[hash % AI_HASHMAP_SIZE];

  while (true) {
#ifdef MM_OPT_UPDATE_MEMO
    for (size_t i=0; i<node->size; i++) {
      if (node->array[i].hash != hash || !compare(board, &node->array[i].board))
        continue;

      assert(node->array[i].depth < depth);

      node->array[i].depth = depth;
      node->array[i].eval = eval;
      return;
    }
#endif

    if (node->size < AI_LL_NODE_SIZE) break;

    if (!node->next) {
      node->next = malloc(sizeof(ai_cache_node_t));
      node = node->next;
      node->next = NULL;
      node->size = 0;
      break;
    }

    node = node->next;
  }

  node->array[node->size++] = (struct memorized_t) { .board=*board, .hash=hash, .depth=depth, .eval=eval, .move=move };
}

// Get if the board was saved for memoization before.
bool try_remember(ai_cache_t* cache, hash_t hash, board_t* board, size_t depth, eval_t* eval, move_t* move) {
  for (ai_cache_node_t* node = cache->memorized[hash % AI_HASHMAP_SIZE]; node != NULL; node = node->next) {
    for (int i=0; i<node->size; i++) {
      struct memorized_t* memorized = &node->array[i];

      if (memorized->hash != hash)
        continue;

      if (compare(board, &memorized->board) && memorized->depth >= depth) {
        *eval = memorized->eval;
        *move = memorized->move;
        return true;
      }
    }
  }

  return false;
}
#endif

void setup_adv_tables() {
  for (int row=0; row<8; row++) {
    for (int col=0; col<8; col++) {
      int topleft_row = row;
      int topleft_col = col;

      if (row >= 4) topleft_row = 7 - row;
      if (col >= 4) topleft_col = 7 - col;

      pos_t pos = to_position(row, col);

      PAWN_ADV_TABLE[pos] = TOPLEFT_PAWN_ADV_TABLE[topleft_row][topleft_col];
      KNIGHT_ADV_TABLE[pos] = TOPLEFT_KNIGHT_ADV_TABLE[topleft_row][topleft_col];
    }
  }
}
