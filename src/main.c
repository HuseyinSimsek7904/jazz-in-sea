#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <getopt.h>
#include <unistd.h>

#include "ai/search.h"

#include "board/board_t.h"
#include "board/piece_t.h"
#include "board/pos_t.h"
#include "move/move_t.h"
#include "io/pp.h"
#include "io/fen.h"
#include "commands/globals.h"


void initialize() {
  if (!load_fen_from_path("board_fen/starting", &game_state, &game_history)) {
    io_error();
    pp_f("error: could not load starting position\n");
    exit(1);
  }

  // Default configurations for interactive and non interactive usages.
  if (isatty(fileno(stdin))) {
    global_options.interactive = true;

    global_options.file_basic = stdout;
    global_options.file_error = stderr;
    global_options.file_info = stdout;
    global_options.file_debug = stderr;

    global_options.exit_if_error = false;

  } else {
    global_options.interactive = false;

    global_options.file_basic = stdout;
    global_options.file_error = stderr;
    global_options.file_info = stderr;
    global_options.file_debug = stderr;

    global_options.exit_if_error = true;
  }

  global_options.ai_depth = 256;
  global_options.ai_time.tv_nsec = 0;
  global_options.ai_time.tv_sec = 2;

  global_options.white_automove = false;
  global_options.black_automove = false;
}

int main(int argc, char **argv) {
  srand(time(NULL));

  initialize();

  // Get the global flags.
  const char* const global_opts = "hsr";

  while (argv[optind]) {
    char c = getopt(argc, argv, global_opts);
    switch (c) {
    case 'h':
      io_info();
      pp_f("\n"
           "                -- Jazz in Sea -- \n"
           "  An AI attempt for the board game Cez written in C\n"
           "\n"
           "    -h  Show this message\n"
           "    -r  Derandomize the output\n"
           "    -s  Silence the information and debug messages\n");
      return 0;
    case 'r':
      srand(0);
      break;
    case 's':
      global_options.file_info = fopen("/dev/null", "w");
      global_options.file_debug = fopen("/dev/null", "w");
      break;
    }
  }

  while (true) {
    fflush(stdout);
    fflush(stderr);

#define ARGV_SIZE 32
#define ARG_BUFFER_SIZE 4096

    char* argv[ARGV_SIZE];
    char arg_buffer[ARG_BUFFER_SIZE];
    int argc = generate_argv(arg_buffer, argv, ARG_BUFFER_SIZE, ARGV_SIZE);

    if (argc <= 0) {
      continue;
    };

    char* command = argv[0];

    // Check for the command name in the command entries.
    size_t command_id=0;
    for (; command_entries[command_id].name != NULL; command_id++) {
      if (!strcmp(command, command_entries[command_id].name)) {
        bool success = command_entries[command_id].function(argc, argv);

        if (!success && global_options.exit_if_error)
          return 1;

        break;
      }
    }

    if (command_entries[command_id].name == NULL) {
      io_error();
      pp_f("error: unknown command '%s'\n", command);

      if (global_options.exit_if_error)
        return 1;
    }
  }
}
