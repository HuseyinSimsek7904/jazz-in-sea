#include "ai/position_evaluation.h"
#include "ai/cache.h"
#include "io/pp.h"
#include "state/board_state_t.h"
#include "board/pos_t.h"
#include "board/board_t.h"

// Generate a short "guessed" evaluation score for a move for move ordering.
// Ignore whether or not pieces are in islands.
eval_t get_short_move_evaluation(board_state_t* state, ai_cache_t* cache, move_t move) {
  piece_t piece = get_piece(state->board, move.from);
  eval_t evaluation;

  switch (get_piece_type(piece)) {
  case MOD_PAWN:
    evaluation = cache->pawn_adv_table[move.to] - cache->pawn_adv_table[move.from];
    break;
  case MOD_KNIGHT:
    evaluation = cache->knight_adv_table[move.to] - cache->knight_adv_table[move.from];
    break;
  default:
    assert(false);
    return 0;
  }

  if (is_capture(move)) {
    switch (get_piece_type(move.capture_piece)) {
    case MOD_PAWN:
      evaluation = cache->pawn_adv_table[move.capture];
      break;
    case MOD_KNIGHT:
      evaluation = cache->knight_adv_table[move.capture];
      break;
    default:
      assert(false);
      return 0;
    }
  }

  switch (get_piece_color(piece)) {
  case MOD_WHITE:
    break;
  case MOD_BLACK:
    evaluation = -evaluation;
    break;
  default:
    assert(false);
    return 0;
  }

  return evaluation;
}

// Generate a full evaluation score for the current board.
int get_board_evaluation(board_state_t* state, ai_cache_t* cache) {
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
