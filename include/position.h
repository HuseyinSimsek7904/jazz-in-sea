#include <stdbool.h>

#define INV_POSITION 0x88

unsigned int to_position(int, int);
int to_row(unsigned int);
int to_col(unsigned int);

int perspective_row(int, bool);
int perspective_col(int, bool);

bool is_valid_pos(unsigned int);

char col_name(int);
char row_name(int);
void print_position(unsigned int);
