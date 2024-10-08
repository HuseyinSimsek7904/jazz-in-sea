#include <stdint.h>

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
  int piece;
} move_t;

void print_move(move_t move);
