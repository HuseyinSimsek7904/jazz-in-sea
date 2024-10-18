#pragma once

#include "board.h"

#define INV_MOVE (move_t) { .from=INV_POSITION }

typedef struct {
  // The position from where the piece moved.
  int from;

  // The position where the piece moved to.
  int to;

  // The position of the piece that the moving piece captured.
  // INV_POSITION if no capture.
  int capture;

  // The type of the piece that the moving piece captured.
  // Does not matter if no capture.
  int capture_piece;
} move_t;

bool is_valid_move(move_t);

void print_move(move_t);

void do_move(board_t*, move_t);
void undo_move(board_t*, move_t);

bool is_capture(move_t);
