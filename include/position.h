#include <stdbool.h>

#define INV_POSITION 0x88

int to_position(int, int);
int to_row(int);
int to_col(int);

int perspective_row(int row, bool reverse);
int perspective_col(int col, bool reverse);

bool is_valid_pos(int);
