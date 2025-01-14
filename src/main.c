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

board_state_t game_state;
history_t game_history;

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
  if (game_state.turn ? !white_automove : !black_automove)
    return;

  // Check if the game ended.
  if (game_state.status != NORMAL) {
    cli_info printf("could not automove, game ended\n");
    cli_info printf("%s\n", board_status_text(game_state.status));

    white_automove = false;
    black_automove = false;
    return;
  }

  // Select one of the moves generated by the AI and make move.
  cli_info printf("automove... ");
  move_t best_moves[256];
  size_t best_moves_length;

  eval_t eval = evaluate(&game_state, &game_history, ai_depth, best_moves, &best_moves_length);
  move_t chosen_move = best_moves[rand() % best_moves_length];

  do_move(&game_state, &game_history, chosen_move);

  cli_info printf("done\n");
  cli_info printf("Played ");
  cli_info print_move(chosen_move);
  cli_info printf(" with evaluation ");
  cli_info print_eval(eval, game_state.board, &game_history);
  cli_info print_board(game_state.board, false);

  make_automove();
}

command(loadfen) {
  optind = 0;
  while (true) {
    int c = getopt(argc, argv, "s:p:");
    switch (c) {
    case -1:
    case '?':
      return;
    case 's':
      load_fen_string(optarg, &game_state, &game_history);
      return;
    case 'p':
      if (!load_fen_from_path(optarg, &game_state, &game_history)) {
        cli_error("could not load FEN\n");
      }
      return;
    }
  }
}

command(savefen) {
  optind = 0;
  while (true) {
    int c = getopt(argc, argv, "sp:");
    switch (c) {
    case '?':
      return;
    case -1:
    case 's':
      {
        char buffer[256];
        get_fen_string(buffer, &game_state);
        printf("%s\n", buffer);
        return;
      }
    case 'p':
      if (!save_fen_to_path(optarg, &game_state)) {
        cli_error("could save load FEN\n");
      }
      return;
    }
  }
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
    print_board(game_state.board, false);
    break;
  case HASH:
    printf("%lx\n", game_state.hash);
    break;
  case ISLANDS:
    print_islands(&game_state, false);
  }
}

command(makemove) {
  expect_n_arguments("makemove", 1);

  move_t move;
  if (!string_to_move(argv[1], game_state.board, &move)) {
    cli_error("invalid move notation '%s'\n", argv[1]);
    return;
  }

  move_t moves[256];
  size_t moves_length = generate_moves(&game_state, moves);

  for (int i=0; i<moves_length; i++) {
    if (compare_move(moves[i], move)) {
      do_move(&game_state, &game_history, move);
      moves_length = generate_moves(&game_state, moves);

      make_automove();
      return;
    }
  }

  cli_error("invalid move\n");
  return;
}

command(undomove) {
  expect_n_arguments("undomove", 0);

  if (!game_history.size) {
    cli_error("no previous move\n");
    return;
  }

  undo_last_move(&game_state, &game_history);
}

command(status) {
  expect_n_arguments("status", 0);

  printf("%s\n", board_status_text(game_state.status));
}

command(allmoves) {
  expect_n_arguments("allmoves", 0);

  move_t moves[256];
  size_t length = generate_moves(&game_state, moves);

  print_moves(moves, length);
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
  move_t best_moves[256];
  size_t best_moves_length;
  eval_t eval = evaluate(&game_state, &game_history, ai_depth, best_moves, &best_moves_length);

  do_move(&game_state, &game_history, best_moves[rand() % best_moves_length]);
  cli_info printf("done\n");
  cli_info print_eval(eval, game_state.board, &game_history);

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
  move_t best_moves[256];
  size_t best_moves_length;
  eval_t eval;

#ifdef MEASURE_EVAL_TIME
  {
    clock_t start = clock();
#endif

    eval = evaluate(&game_state, &game_history, ai_depth, best_moves, &best_moves_length);
    cli_info printf("done\n");

#ifdef MEASURE_EVAL_TIME
    cli_info printf("info: took %ldms\n", (clock() - start) / (CLOCKS_PER_SEC / 1000));
  }
#endif

#ifdef MEASURE_EVAL_COUNT
  cli_info printf("info: called _evaluate %d times.\n",
                  get_evaluate_count());
  cli_info printf("info: cut %d branches.\n",
                  get_ab_branch_cut_count());
  cli_info printf("info: found %d (%d %%) different game ends.\n",
                  get_game_end_count(),
                  get_game_end_count() * 100 / get_evaluate_count());
  cli_info printf("info: found total %d (%d %%) leaves.\n",
                  get_leaf_count(),
                  get_leaf_count() * 100 / get_evaluate_count());

#ifdef MM_OPT_MEMOIZATION
  cli_info printf("info: in total, used %d (%d %%) transposition tables entries.\n",
                  get_tt_saved_count(),
                  get_tt_saved_count() * 100 / AI_HASHMAP_SIZE);
  cli_info printf("info: remembered %d (%d %% per call, %d %% per entry) times.\n",
                  get_tt_remember_count(),
                  get_tt_remember_count() * 100 / get_evaluate_count(),
                  get_tt_remember_count() * 100 / get_tt_saved_count());
  cli_info printf("info: overwritten the same board %u (%u %%) times.\n",
                  get_tt_overwritten_count(),
                  get_tt_overwritten_count() * 100 / get_tt_saved_count());
  cli_info printf("info: rewritten a different board %u (%u %%) times.\n",
                  get_tt_rewritten_count(),
                  get_tt_rewritten_count() * 100 / get_tt_saved_count());
#endif


#endif

  switch (evaluation_type) {
  case LIST:
    print_moves(best_moves, best_moves_length);
    printf("\n");
    break;
  case RANDOM_MOVE:
    print_move(best_moves[rand() % best_moves_length]);
    printf("\n");
    break;
  case EVAL_TEXT:
    print_eval(eval, game_state.board, &game_history);
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

  place_piece(&game_state, &game_history, pos, piece);
}

command(removeat) {
  expect_n_arguments("removeat", 1);

  pos_t pos;
  if (!string_to_position(argv[1], &pos)) {
    printf("%s\n", argv[1]);
    cli_error("invalid position\n");
    return;
  }

  remove_piece(&game_state, &game_history, pos);
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
  size_t length = generate_moves(&game_state, moves);
  for (size_t i=0; i<length; i++) {
    do_move(&game_state, &game_history, moves[i]);
    branches += count_branches(depth - 1);
    undo_last_move(&game_state, &game_history);
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
               "    undomove          undo last move\n"
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
      help_command(undomove, "undomove: undo the last move made\n")
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

  if (feof(stdin))
    exit(0);

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
          return -1;
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
  if (!load_fen_from_path("board_fen/starting", &game_state, &game_history)) {
    cli_error("could not load starting position\n");
    exit(1);
  }
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
    fflush(stdout);
    fflush(stderr);
    if (feof(stdin)) return 0;

    char* argv[32];
    char arg_buffer[512];
    int argc = generate_argv(arg_buffer, argv);

    if (argc <= 0) continue;

    char* command = argv[0];

    // Check for the commands.
    if (false) {}
    expect_command(help)
      expect_command(loadfen)
      expect_command(savefen)
      expect_command(show)
      expect_command(makemove)
      expect_command(undomove)
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
