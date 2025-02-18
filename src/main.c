/*
This file is part of JazzInSea.

JazzInSea is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

JazzInSea is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
JazzInSea. If not, see <https://www.gnu.org/licenses/>.
*/

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "ai/search.h"
#include "board/board_t.h"
#include "board/piece_t.h"
#include "board/pos_t.h"
#include "commands/commands.h"
#include "commands/globals.h"
#include "io/fen.h"
#include "io/pp.h"
#include "move/generation.h"
#include "move/move_t.h"

void print_help_message(const char *executable) {
  io_info();
  pp_f("\n"
       "                -- Jazz in Sea -- \n"
       "  An open source Cez engine written completely in C\n"
       "\n"
       "Usage: %s [OPTION]... [COMMAND]...\n"
       "\n"
       "Run commands and start interactive mode.\n"
       "\n"
       "    -h          Show this message\n"
       "    -r          Set the random seed to 0\n"
       "    -s          Do not print 'info' messages\n"
       "    -n          Do not accept commands via stdin (do not start "
       "interactive mode), only command line\n"
       "    -i PATH     Dump 'info' messages to PATH, if PATH is '%' then to "
       "stderr\n"
       "    -d PATH     Dump 'debug' messages to PATH, if PATH is '%' then to "
       "stderr\n",
       executable);
}

void parse_command_line_args(int argc, char **argv) {
  const char *executable = argv[0];
  const char *global_opts = "hrnsi:d:";

  while (true) {
    char c = getopt(argc, argv, global_opts);
    switch (c) {
    case -1:
      return;

    case '?':
      print_help_message(global_options.executable);
      exit(0);

    case 'h':
      print_help_message(executable);
      exit(1);

    case 'r':
      srand(0);
      break;

    case 's':
      global_options.file_info = fopen("/dev/null", "w");
      break;

    case 'n':
      global_options.accept_stdin = false;
      break;

    case 'i':
      if (!strcmp(optarg, "%")) {
        global_options.file_info = stderr;
      } else {
        global_options.file_info = fopen(optarg, "a");
        if (global_options.file_info < 0) {
          pp_f("error: could not open file '%s'", optarg);
          exit(1);
        }
      }
      break;

    case 'd':
      if (!strcmp(optarg, "%")) {
        global_options.file_debug = stderr;
      } else {
        global_options.file_debug = fopen(optarg, "a");
        if (global_options.file_debug < 0) {
          pp_f("error: could not open file '%s'", optarg);
          exit(1);
        }
      }
      break;
    }
  }
}

void execute_command(int command_argc, char **command_argv) {
  if (command_argc <= 0) {
    return;
  };

  char *command = command_argv[0];

  // Check for the command name in the command entries.
  size_t command_id = 0;
  for (; command_entries[command_id].name != NULL; command_id++) {
    if (!strcmp(command, command_entries[command_id].name)) {
      bool success =
          command_entries[command_id].function(command_argc, command_argv);

      if (!success && global_options.exit_if_error)
        exit(1);

      break;
    }
  }

  if (command_entries[command_id].name == NULL) {
    io_error();
    pp_f("error: unknown command '%s'\n", command);

    if (global_options.exit_if_error)
      exit(1);
  }
}

void initialize(int argc, char **argv) {
  srand(time(NULL));

  global_options = (options_t){
      .executable = argv[0],
      .accept_stdin = true,

      .ai_tt_size = 0x20000,
      .ai_depth = 256,
      .ai_time.tv_nsec = 0,
      .ai_time.tv_sec = 2,

      .white_automove = false,
      .black_automove = false,
      .file_basic = stdout,
      .file_error = stderr,
      .file_info = stderr,
      .file_debug = fopen("/dev/null", "w"),
  };

  parse_command_line_args(argc, argv);

  // Default configurations for interactive and non interactive usages.
  if (isatty(fileno(stdin))) {
    global_options.interactive = true;

    global_options.exit_if_error = false;

  } else {
    global_options.interactive = false;

    global_options.exit_if_error = true;
  }

  if (!load_fen_string("np4PN/pp4PP/8/8/8/8/PP4pp/NP4pn w", &game_state, &game_history)) {
    io_error();
    pp_f("error: could not load starting position\n");
    exit(1);
  }
}

int main(int argc, char **argv) {
  initialize(argc, argv);

#define ARGV_SIZE 32
#define ARG_BUFFER_SIZE 4096

  char command_arg_buffer[ARG_BUFFER_SIZE];
  char *command_argv[ARGV_SIZE];
  int command_argc;

  for (int command_optind = optind; command_optind < argc; command_optind++) {
    io_info();
    pp_f("[filled from argument] -- %s\n", argv[command_optind]);

    command_argc = generate_argv(argv[command_optind], command_arg_buffer,
                                 command_argv, ARG_BUFFER_SIZE, ARGV_SIZE);
    execute_command(command_argc, command_argv);
  }

  if (!global_options.accept_stdin)
    return 0;

  while (true) {
    fflush(stdout);
    fflush(stderr);

#define COMMAND_INPUT_BUFFER_SIZE 4096
    char command_input_buffer[COMMAND_INPUT_BUFFER_SIZE];

    io_info();

    if (global_options.interactive) {
      pp_f("> ");
    }

    char *result =
        fgets(command_input_buffer, COMMAND_INPUT_BUFFER_SIZE, stdin);

    if (!global_options.interactive) {
      if (result != NULL) {
        pp_f("[filled from pipe] -- %s", result);
      }
    }

    if (feof(stdin)) {
      pp_f("[end of pipe]\n");
      fclose(global_options.file_basic);
      fclose(global_options.file_error);
      fclose(global_options.file_info);
      fclose(global_options.file_debug);
      return 0;
    }

    command_argc = generate_argv(command_input_buffer, command_arg_buffer,
                                 command_argv, ARG_BUFFER_SIZE, ARGV_SIZE);
    execute_command(command_argc, command_argv);
  }
}
