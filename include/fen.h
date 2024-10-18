#ifndef _FEN_H
#define _FEN_H

#include <stdbool.h>

#include "board.h"

#define DEFAULT_BOARD "np4PN/pp4PP/8/8/8/8/PP4pp/NP4pn w"

bool load_fen(const char*, board_t*);

#endif
