/*
This file is part of JazzInSea.

JazzInSea is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

JazzInSea is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with JazzInSea. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _COMMANDS_GLOBALS_H
#define _COMMANDS_GLOBALS_H

#include "commands/commands.h"
#include "state/board_state_t.h"
#include "state/history.h"

#include <stdbool.h>
#include <stdio.h>
#include <time.h>

typedef struct {
  char* executable;

  struct timespec ai_time;
  size_t ai_depth;

  bool white_automove;
  bool black_automove;

  bool exit_if_error;
  bool interactive;
  bool accept_stdin;

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
