#include <assert.h>
#include <bits/getopt_core.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <getopt.h>

#include "ai.h"
#include "board.h"
#include "position.h"
#include "move.h"
#include "rules.h"
#include "fen.h"
#include "io.h"

#define cli            if (cli_logs)
#define cli_error(...) if (cli_logs) fprintf(stderr, "error: " __VA_ARGS__); else exit(1);

#define expect_n_arguments(s, n)                                        \
  if (argc != n+1) {                                                    \
    printf("error: command '%s' expects exactly %u argument.\n", s, n); \
    return;                                                             \
  }

// API states
board_t game_board;

// API flags
bool cli_logs = false;


void initialize() {
  load_fen(DEFAULT_BOARD, &game_board);
}

void command_help(int argc, char** argv) {
  if (argc == 1) {
    cli printf("commands:\n"
               "    help              get information about commands\n"
               "    loadfen           load a board from its FEN\n"
               "    show              show the current board\n"
               "    makemove          make a move");

  } else if (argc == 2) {
    if (!strcmp(argv[1], "help")) {
      cli printf("help [<command>]: list or get information about commands\n");

    } else if (!strcmp(argv[1], "loadfen")) {
      cli printf("loadfen <fen>: load a board configuration from a FEN string\n");

    } else if (!strcmp(argv[1], "show")) {
      cli printf("show: print the current board configuration\n");

    } else if (!strcmp(argv[1], "makemove")) {
      cli printf("makemove <move>: make a move on the current board\n");

    } else {
      cli_error("unknown command\n");
      return;
    }

  } else {
    cli_error("command 'help' requires either 0 or 1 arguments.\n");
    return;
  }
}

void command_loadfen(int argc, char** argv) {
  expect_n_arguments("loadfen", 1);

  board_t new_board;
  if (!load_fen(argv[1], &new_board)) {
    cli_error("invalid fen\n");
    return;
  }

  copy_board(&new_board, &game_board);
}

void command_show(int argc, char** argv) {
  expect_n_arguments("show", 0);

  cli print_board(&game_board, false);
}

void command_makemove(int argc, char** argv) {
  expect_n_arguments("makemove", 1);

  move_t move;
  if (!string_to_move(argv[1], &game_board, &move)) {
    cli_error("invalid move notation\n");
    return;
  }

  move_t moves[256];
  size_t length = generate_moves(&game_board, moves);

  for (int i=0; i<length; i++) {
    if (cmp_move(moves[i], move)) {
      do_move(&game_board, move);
      return;
    }
  }

  cli_error("invalid move\n");
  return;
}

bool is_whitespace(char c) { return c == ' ' || c == '\t' || c == '\n'; }

#define ARG_BUFFER_SIZE 512
#define ARGV_SIZE 32

int generate_argv(char* arg_buffer, char** argv) {
  // Get the command and parse it.
  char buffer[256];

  cli printf("> ");

  fgets(buffer, sizeof(buffer), stdin);

  int argc = 0;
  char* buffer_ptr = buffer;
  char* arg_buffer_ptr = arg_buffer;
  while (true) {
    // Ignore the whitespaces.
    char c;
    do {
      c = *buffer_ptr++;
    } while (is_whitespace(c));

    // If reached the end of the string, finish parsing.
    if (c == '\0') break;

    char quote = '\0';
    if (c == '\'' || c == '"'){
      quote = c;
      c = *buffer_ptr++;
    }

    // Copy the string from the buffer to arg buffer.
    if (argc > ARGV_SIZE) {
      cli_error("argv overflow\n");
      return -1;
    }
    argv[argc++] = arg_buffer_ptr;

    while (quote ? c != quote : !is_whitespace(c)){
      if (c == '\0') {
        if (quote) {
          cli_error("unterminated quote\n");
        }

        break;
      }

      // Check for buffer overflow and move the character.
      if (arg_buffer_ptr > arg_buffer + ARG_BUFFER_SIZE) {
        cli_error("arg buffer overflow\n");
        return -1;
      }
      *arg_buffer_ptr++ = c;

      c = *buffer_ptr++;
    }

    // Check for buffer overflow and move the character.
    if (arg_buffer_ptr > arg_buffer + ARG_BUFFER_SIZE) {
      cli_error("arg buffer overflow\n");
      return -1;
    }
    *arg_buffer_ptr++ = '\0';
  }

  // After all of the buffers are filled, add a null ptr to the arg_ptr_buffer.
  if (argc > ARGV_SIZE) {
    cli_error("argv overflow\n");
    return -1;
  }

  argv[argc] = 0;
  return argc;
}

int main(int argc, char** argv) {
  initialize();
  srand(time(NULL));

  // Get the global flags.
  const char* const global_opts = "h";

  while (argv[optind]) {
    char c = getopt(argc, argv, global_opts);
    switch (c) {
    case 'h':
      printf("\n"
             "                -- Jazz in Sea -- \n"
             "  An AI attempt for the board game Cez written in C\n"
             "\n"
             "run `%s` without any options to start the cli.\n"
             , argv[0]);


      return 0;
    }
  }

  while (true) {
    char* argv[32];
    char arg_buffer[512];
    int argc = generate_argv(arg_buffer, argv);

    if (argc < 0) continue;

    char* command = argv[0];

    if (!strcmp(command, "help")) {
      command_help(argc, argv);

    } else if (!strcmp(command, "loadfen")) {
      command_loadfen(argc, argv);

    } else if (!strcmp(command, "show")) {
      command_show(argc, argv);

    } else if (!strcmp(command, "makemove")) {
      command_makemove(argc, argv);

    } else {
      cli_error("unknown command '%s'\n", command);
      continue;
    }
  }
}
