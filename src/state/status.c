#include "state/status.h"
#include "board/status_t.h"
#include "state/history.h"

// Get the board status.
void generate_board_status(board_state_t* state, history_t* history) {
  // Check for insufficient material.
  if (!state->white_count) {
    state->status = state->black_count ? BLACK_WON_BY_INSUF_MAT : DRAW_BY_INSUF_MAT;
    return;
  }

  if (!state->black_count) {
    state->status = WHITE_WON_BY_INSUF_MAT;
    return;
  }

  // Check for islands.
  if (state->white_count == state->white_island_count) {
    state->status = (state->black_count == state->black_island_count) ? DRAW_BY_BOTH_ISLANDS : WHITE_WON_BY_ISLANDS;
    return;
  }

  if (state->black_count == state->black_island_count) {
    state->status = BLACK_WON_BY_ISLANDS;
    return;
  }

  // Check for draw by repetition.
  if (check_for_repetition(history, state->hash, 2)) {
    state->status = DRAW_BY_REPETITION;
    return;
  }

  state->status = NORMAL;
}
