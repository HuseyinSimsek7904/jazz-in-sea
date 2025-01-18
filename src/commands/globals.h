#ifndef _COMMANDS_GLOBALS_H
#define _COMMANDS_GLOBALS_H

#include "commands/commands.h"
#include "state/board_state_t.h"
#include "state/history.h"

#include <stdbool.h>
#include <stdio.h>

typedef struct {
  size_t ai_depth;

  bool white_automove;
  bool black_automove;

  bool exit_if_error;
  bool interactive;

  FILE* file_basic;
  FILE* file_error;
  FILE* file_debug;
  FILE* file_info;

  FILE* current_file;
} options_t;

extern options_t global_options;
extern command_entry_t command_entries[];

extern board_state_t game_state;
extern history_t game_history;

#endif
