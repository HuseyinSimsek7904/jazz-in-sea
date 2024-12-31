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


// -- API states and flags --
size_t ai_depth = 8;
bool cli_logs = true;
bool be_descriptive = false;
board_t game_board;
state_cache_t game_state;
move_t game_all_moves[256];
size_t game_all_moves_length;
bool black_automove;
bool white_automove;


// -- Commands --
#define command(name)                           \
  void command_ ## name(int argc, char** argv)  \

#define cli            if (cli_logs)
#define cli_info       if (be_descriptive) cli
#define cli_error(...) if (cli_logs) fprintf(stderr, "error: " __VA_ARGS__); else exit(1);

#define expect_n_arguments(s, n)                                        \
  if (argc != n + 1) {                                                  \
    printf("error: command '%s' expects exactly %u argument.\n", s, n); \
    return;                                                             \
  }

void make_automove() {
  // Check if the current player should be automoved.
  if (game_board.turn ? !white_automove : !black_automove)
    return;

  // Check if the game ended.
  if (game_state.status != NORMAL) {
    cli_info printf("could not automove, game ended\n");

    white_automove = false;
    black_automove = false;
    return;
  }

  // Select one of the moves generated by the AI and make move.
  cli_info printf("automove... ");
  move_t moves[256];
  eval_t eval;

  size_t length = evaluate(&game_board, &game_state, ai_depth, moves, &eval);

  do_move(&game_board, &game_state, moves[rand() % length]);
  game_all_moves_length = generate_moves(&game_board, game_all_moves);
  cli_info printf("done\n");

  make_automove();
}

command(loadfen) {
  expect_n_arguments("loadfen", 1);

  board_t new_board;
  if (!load_fen(argv[1], &game_state, &new_board)) {
    cli_error("invalid fen\n");
    return;
  }

  game_board = new_board;
  generate_state_cache(&game_board, &game_state);
  game_all_moves_length = generate_moves(&game_board, game_all_moves);

  cli_info printf("successfully loaded from fen\n");
  cli_info print_board(&game_board, false);

  make_automove();
}

command(savefen) {
  expect_n_arguments("savefen", 0);

  char buffer[256];
  save_fen(buffer, &game_board);
  printf("%s\n", buffer);
}

command(show) {
  enum { BOARD, HASH, ISLANDS } show_type = BOARD;

  optind = 0;
  while (true) {
    int c = getopt(argc, argv, "bhi");
    switch (c) {
    case -1:
      goto end_of_parsing;
    case '?':
      return;
    case 'b':
      show_type = BOARD;
      break;
    case 'h':
      show_type = HASH;
      break;
    case 'i':
      show_type = ISLANDS;
      break;
    }
  }

 end_of_parsing:
  switch (show_type) {
  case BOARD:
    print_board(&game_board, false);
    break;
  case HASH:
    printf("%u\n", game_state.hash);
    break;
  case ISLANDS:
    print_islands(&game_board, game_state, false);
  }
}

command(makemove) {
  expect_n_arguments("makemove", 1);

  move_t move;
  if (!string_to_move(argv[1], &game_board, &move)) {
    cli_error("invalid move notation '%s'\n", argv[1]);
    return;
  }

  for (int i=0; i<game_all_moves_length; i++) {
    if (cmp_move(game_all_moves[i], move)) {
      do_move(&game_board, &game_state, move);
      game_all_moves_length = generate_moves(&game_board, game_all_moves);

      make_automove();
      return;
    }
  }

  cli_error("invalid move\n");
  return;
}

command(status) {
  expect_n_arguments("status", 0);

  printf("%s\n", board_status_text(game_state.status));
}

command(allmoves) {
  expect_n_arguments("allmoves", 0);

  print_moves(game_all_moves, game_all_moves_length);
  printf("\n");
}

command(automove) {
  expect_n_arguments("automove", 2);

  bool set;
  if (!strcmp(argv[2], "on")) {
    set = true;
  } else if (!strcmp(argv[2], "off")) {
    set = false;
  } else {
    cli_error("second argument must be either 'on' or 'off'\n");
    return;
  }

  if (!strcmp(argv[1], "white")) {
    white_automove = set;

  } else if (!strcmp(argv[1], "black")) {
    black_automove = set;

  } else if (!strcmp(argv[1], "both")) {
    white_automove = set;
    black_automove = set;

  } else {
    cli_error("first argument must be either 'white', 'black' or 'both'\n");
  }

  make_automove();
}

command(playai) {
  expect_n_arguments("playai", 0);

  // Check if the game ended.
  if (game_state.status != NORMAL) {
    cli_info printf("could not play any moves, game ended\n");
    return;
  }

  // Select one of the moves generated by the AI and make move.
  cli_info printf("playing... ");
  move_t moves[256];
  eval_t eval;

  size_t length = evaluate(&game_board, &game_state, ai_depth, moves, &eval);

  do_move(&game_board, &game_state, moves[rand() % length]);
  game_all_moves_length = generate_moves(&game_board, game_all_moves);
  cli_info printf("done\n");

  make_automove();
}

command(evaluate) {
  enum { LIST, RANDOM_MOVE, EVAL_TEXT } evaluation_type = EVAL_TEXT;

  optind = 0;
  while (true) {
    int c = getopt(argc, argv, "trl");
    switch (c) {
    case -1:
      goto end_of_parsing;
    case '?':
      return;
    case 't':
      evaluation_type = EVAL_TEXT;
      break;
    case 'r':
      evaluation_type = RANDOM_MOVE;
      break;
    case 'l':
      evaluation_type = LIST;
      break;
    }
  }

 end_of_parsing:
  // Check if the game ended.
  if (game_state.status != NORMAL) {
    cli printf("game ended\n");
    return;
  }

  // Print the calculated evaluation of the AI.
  cli_info printf("evaluating... ");
  move_t moves[256];
  eval_t eval;

  size_t length = evaluate(&game_board, &game_state, ai_depth, moves, &eval);
  cli_info printf("done\n");

#ifdef EVALCOUNT
  cli_info printf("called _evaluate %d times.\n", get_evaluate_count());
  cli_info printf("remembered %d (%d %%) times.\n", get_remember_count(), get_remember_count() * 100 / get_evaluate_count());
  cli_info printf("found %d (%d %%) different game ends.\n", get_game_end_count(), get_game_end_count() * 100 / get_evaluate_count());
  cli_info printf("found total %d (%d %%) leaves.\n", get_leaf_count(), get_leaf_count() * 100 / get_evaluate_count());
#endif

  switch (evaluation_type) {
  case LIST:
    print_moves(moves, length);
    printf("\n");
    break;
  case RANDOM_MOVE:
    print_move(moves[rand() % length]);
    printf("\n");
    break;
  case EVAL_TEXT:
    print_eval(eval, &game_board);
    break;
  }
}

command(placeat) {
  expect_n_arguments("placeat", 2);

  pos_t pos;
  char piece = argv[2][0];
  if (!string_to_position(argv[1], &pos)) {
    printf("%s\n", argv[1]);
    cli_error("invalid position\n");
    return;
  }

  place_piece(&game_board, &game_state, pos, piece);
  game_all_moves_length = generate_moves(&game_board, game_all_moves);
}

command(removeat) {
  expect_n_arguments("removeat", 1);

  pos_t pos;
  if (!string_to_position(argv[1], &pos)) {
    printf("%s\n", argv[1]);
    cli_error("invalid position\n");
    return;
  }

  remove_piece(&game_board, &game_state, pos);
  game_all_moves_length = generate_moves(&game_board, game_all_moves);
}

command(aidepth) {
  switch (argc) {
  case 1:
    cli printf("%zu\n", ai_depth);
    break;
  case 2:
    ai_depth = atoi(argv[1]);
    break;
  default:
    cli_error("command 'aidepth' expects 0 or 1 argument.\n");
    return;
  }
}

static inline size_t count_branches(size_t depth) {
  if (!depth) return 1;

  size_t branches = 0;

  move_t moves[256];
  size_t length = generate_moves(&game_board, moves);
  for (size_t i=0; i<length; i++) {
    do_move(&game_board, &game_state, moves[i]);

    branches += count_branches(depth - 1);

    undo_move(&game_board, &game_state, moves[i]);
  }

  return branches;
}

command(test) {
  size_t depth = 0;
  enum { LEAF_COUNT } test = LEAF_COUNT;

  optind = 0;
  while (true) {
    int c = getopt(argc, argv, "l:");
    switch (c) {
    case -1:
      goto end_of_parsing;
    case '?':
      return;
    case 'l':
      depth = atoi(optarg);
      test = LEAF_COUNT;
      break;
    }
  }

 end_of_parsing:
  switch (test) {
  case LEAF_COUNT:
    printf("%zu\n", count_branches(depth));
    break;
  }
}

#define help_command(command_name, ...)         \
  else if (!strcmp(argv[1], #command_name)) {   \
    cli printf(__VA_ARGS__);                    \
  }                                             \

command(help) {
  if (argc == 1) {
    cli printf("cli commands (these will not do anything if run with '-s'):\n"
               "    help              get information about commands\n"
               "    show              show the current board\n"
               "\n"
               "board commands:\n"
               "    loadfen           load a board from its FEN\n"
               "    savefen           get the FEN string of the current board\n"
               "    makemove          make a move\n"
               "    automove          set or reset auto play by AI\n"
               "    status            get the status information of the current board\n"
               "    allmoves          list all of the available moves at the current board\n"
               "    placeat           place a piece on the board\n"
               "    removeat          remove a piece on the board\n"
               "\n"
               "ai commands:\n"
               "    aidepth           set the depth of the AI search\n"
               "    playai            play one of the moves that AI generates\n"
               "    evaluate          get AI evaluation on the board\n"
               "\n"
               "test commands:\n"
               "    test              produce a test information\n"
               );

  } else if (argc == 2) {
    if (false) { }
    // CLI commands
    help_command(help, "help [<command>]: list or get information about commands\n")
      help_command(show, "show: print the current board configuration\n")

      // Board commands
      help_command(loadfen, "loadfen <fen>: load a board configuration from a FEN string\n")
      help_command(savefen, "savefen: get the FEN string of the current board configuration\n")
      help_command(makemove, "makemove <move>: make a move on the current board\n")
      help_command(automove, "automove: set or reset auto play mode by AI\n")
      help_command(status, "status: print the status of the board\n")
      help_command(allmoves, "allmoves: list all of the available moves at the current board\n")
      help_command(placeat, "placeat <pos> <piece>: place PIECE on the board at POS\n")
      help_command(placeat, "removeat <pos>: remove the PIECE on the board\n")

      // AI commands
      help_command(aidepth, "aidepth [<depth>]: get or set the AI's searching depth\n")
      help_command(playai, "playai: play a randomly selected move that AI generated\n")
      help_command(evaluate, "evaluate: get AI evaluation on the current board\n")

      // Test commands
      help_command(test, "test: produce test information\n")

    else {
      cli_error("unknown command\n");
      return;
    }

  } else {
    cli_error("command 'help' requires either 0 or 1 arguments.\n");
    return;
  }
}


// -- Parsing command arguments --
bool is_whitespace(char c) { return c == ' ' || c == '\t' || c == '\n'; }

#define ARG_BUFFER_SIZE 512
#define ARGV_SIZE 32

int generate_argv(char* arg_buffer, char** argv) {
  // Get the command and parse it.
  char buffer[256];

  cli printf("> ");

  if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
    cli printf("\n");
  }

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


void initialize() {
  load_fen(DEFAULT_BOARD, &game_state, &game_board);
  generate_state_cache(&game_board, &game_state);

  game_all_moves_length = generate_moves(&game_board, game_all_moves);
}

#define expect_command(command_name)            \
  else if (!strcmp(command, #command_name))     \
    command_ ## command_name(argc, argv);       \

int main(int argc, char** argv) {
  // Get the global flags.
  const char* const global_opts = "hsdr";

  srand(time(NULL));

  while (argv[optind]) {
    char c = getopt(argc, argv, global_opts);
    switch (c) {
    case 'h':
      printf("\n"
             "                -- Jazz in Sea -- \n"
             "  An AI attempt for the board game Cez written in C\n"
             "\n"
             "    -h  Show this message\n"
             "    -s  Silence the CLI like log messages\n"
             "    -d  Be more descriptive (not effective if used with -s)\n"
             "    -r  Derandomize the output\n");
      return 0;
    case 's':
      cli_logs = false;
      break;
    case 'd':
      be_descriptive = true;
      break;
    case 'r':
      srand(0);
      break;
    }
  }

  initialize();

  while (true) {
    char* argv[32];
    char arg_buffer[512];
    int argc = generate_argv(arg_buffer, argv);
    if (feof(stdin)) return 0;

    if (!argc) continue;

    char* command = argv[0];

    // Check for the commands.
    if (false) {}
    expect_command(help)
      expect_command(loadfen)
      expect_command(savefen)
      expect_command(show)
      expect_command(makemove)
      expect_command(status)
      expect_command(allmoves)
      expect_command(automove)
      expect_command(playai)
      expect_command(evaluate)
      expect_command(placeat)
      expect_command(removeat)
      expect_command(aidepth)
      expect_command(test)
    else {
      cli_error("unknown command '%s'\n", command);
      continue;
    }
  }
}
