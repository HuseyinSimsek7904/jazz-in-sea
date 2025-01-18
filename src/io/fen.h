#ifndef _IO_FEN_H
#define _IO_FEN_H

#include "board/board_t.h"
#include "state/board_state_t.h"
#include "state/history.h"

#include <stdbool.h>

#define DEFAULT_BOARD "np4PN/pp4PP/8/8/8/8/PP4pp/NP4pn w"

bool load_fen_string(const char *, board_state_t *, history_t *);
char *get_fen_string(char *, board_state_t *);

bool load_fen_from_path(const char *, board_state_t *, history_t *);
bool save_fen_to_path(const char *, board_state_t *);

#endif
