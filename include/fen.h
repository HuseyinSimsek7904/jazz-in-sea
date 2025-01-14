#ifndef _FEN_H
#define _FEN_H

#include <stdbool.h>

#include "board.h"
#include "rules.h"

#define DEFAULT_BOARD "np4PN/pp4PP/8/8/8/8/PP4pp/NP4pn w"

bool load_fen_string(const char *, board_t *, state_cache_t *, history_t *);
char *get_fen_string(char *, board_t *);

bool load_fen_from_path(const char *, board_t *, state_cache_t *, history_t *);
bool save_fen_to_path(const char *, board_t *);

#endif
