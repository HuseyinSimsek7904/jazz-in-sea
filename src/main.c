#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <getopt.h>
#include <unistd.h>

#include "io.h"
#include "ai.h"
#include "board.h"
#include "options.h"
#include "position.h"
#include "move.h"
#include "rules.h"
#include "fen.h"


// -- API states flags --

board_state_t game_state;
history_t game_history;

// -- Commands --
#define command(name, simple, usage)                    \
  const char* command_ ## name ## _simple = simple;     \
  const char* command_ ## name ## _usage = usage;       \
  bool command_ ## name(int argc, char** argv)

void make_automove() {
  // Check if the current player should be automoved.
  if (game_state.turn ? !global_options.white_automove : !global_options.black_automove)
    return;

  // Check if the game ended.
  if (game_state.status != NORMAL) {
    io_error();
    pp_f("error: could not automove, game ended\n");
    io_info();
    pp_f("%s\n", board_status_text(game_state.status));

    global_options.white_automove = false;
    global_options.black_automove = false;
    return;
  }

  // Select one of the moves generated by the AI and make move.
  io_info();
  pp_f("automove... ");
  move_t best_moves[256];
  size_t best_moves_length;

  eval_t eval = evaluate(&game_state, &game_history, global_options.ai_depth, best_moves, &best_moves_length);
  move_t chosen_move = best_moves[rand() % best_moves_length];

  io_info();
  pp_f("done\n");
  pp_f("Best move ");
  pp_move(chosen_move);
  pp_f(" with evaluation ");
  pp_eval(eval, game_state.board, &game_history);

  do_move(&game_state, &game_history, chosen_move);

  io_info();
  pp_board(game_state.board, false);

  make_automove();
}

command(loadfen,
        "Load a board position using FEN",
        "Usage: loadfen FEN\n"
        "   or: loadfen PATH -f\n"
        "\n"
        "Load a board configuration from FEN string.\n"
        "\n"
        "  -f            Load FEN from PATH\n"
        ) {

  bool from_file = false;

  optind = 0;
  while (true) {
    int c = getopt(argc, argv, "f");
    switch (c) {
    case '?':
      return false;
    case 'f':
      from_file = true;
      break;
    case -1:
      if (optind >= argc) {
        io_error();
        pp_f("error: 'loadfen' requires an argument\n");
        return false;
      }

      if (from_file) {
        if (!load_fen_from_path(argv[optind], &game_state, &game_history)) {
          io_error();
          pp_f("error: could not load FEN string from file\n");
          return false;
        }
      } else {
        if (!load_fen_string(argv[optind], &game_state, &game_history)) {
          io_error();
          pp_f("error: could not load FEN string\n");
          return false;
        }
      }
      io_info();
      pp_board(game_state.board, false);
      return true;
    }
  }
}

command(savefen,
        "Print the board position FEN",
        "\n"
        "Usage: savefen"
        "\n"
        "Print the current board configuration.\n"
        "\n"
        "  -f PATH       Save the FEN to PATH\n"
        ) {

  bool to_file = false;

  optind = 0;
  while (true) {
    int c = getopt(argc, argv, "f");
    switch (c) {
    case '?':
      return false;
    case 'f':
      to_file = true;
      break;
    case -1:
      if (to_file) {
        if (!save_fen_to_path(argv[optind], &game_state)) {
          io_error();
          pp_f("error: could not load FEN string from file\n");
          return false;
        }
      } else {
        char buffer[256];
        get_fen_string(buffer, &game_state);
        io_basic();
        pp_f("%s\n", buffer);
      }
      return true;
    }
  }
}

command(show,
        "Print the current board position",
        "Usage: show [OPTION]...\n"
        "\n"
        "Print the current board position.\n"
        "\n"
        "  -h            Print the hash value of the board instead\n"
        "  -i            Print the island table instead\n"
        ) {

  enum { BOARD, HASH, ISLANDS } show_type = BOARD;

  optind = 0;
  while (true) {
    int c = getopt(argc, argv, "bhi");
    switch (c) {
    case '?':
      return false;
    case -1:
      goto end_of_parsing;
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
    io_basic();
    pp_board(game_state.board, false);
    return true;
  case HASH:
    io_basic();
    pp_f("%lx\n", game_state.hash);
    return true;
  case ISLANDS:
    io_basic();
    pp_islands(&game_state, false);
    return true;
  }

  // Dummy return
  return false;
}

command(makemove,
        "Make a move",
        "Usage: makemove MOVE\n"
        "\n"
        "Try to make MOVE if it is a valid move.\n"
        ) {

  move_t move;
  if (!string_to_move(argv[1], game_state.board, &move)) {
    io_error();
    pp_f("error: invalid move notation '%s'\n", argv[1]);
    return false;
  }

  move_t moves[256];
  size_t moves_length = generate_moves(&game_state, moves);

  for (int i=0; i<moves_length; i++) {
    if (compare_move(moves[i], move)) {
      do_move(&game_state, &game_history, move);
      moves_length = generate_moves(&game_state, moves);

      make_automove();
      return true;
    }
  }

  io_error();
  pp_f("error: invalid move\n");
  return false;
}

command(undomove,
        "Undo the last move in history",
        "Usage: undomove\n"
        "\n"
        "Undo the last move in history, if there exists one.\n"
        ) {

  if (!game_history.size) {
    io_error();
    pp_f("error: no previous move\n");
    return false;
  }

  undo_last_move(&game_state, &game_history);
  return true;
}

command(status,
        "Print the current board status",
        "Usage: status\n"
        "\n"
        "Print the current board status.\n"
        ) {

  io_info();
  pp_f("%s\n", board_status_text(game_state.status));
  return true;
}

command(allmoves,
        "Print the available moves on the current board",
        "Usage: allmoves\n"
        "\n"
        "Print the available moves on the current board.\n"
        ) {

  move_t moves[256];
  size_t length = generate_moves(&game_state, moves);

  io_basic();
  pp_moves(moves, length);
  pp_f("\n");
  return true;
}

command(automove,
        "Set or unset the automove flag",
        "Usage: automove [OPTION]..."
        "\n"
        "Set the automove flag for both players. When a players automove flag is on and it is their move to play, the AI automatically generates and plays a random move.\n"
        "\n"
        "  -d            Unset the flags instead of setting them\n"
        "  -w            Set or unset only the white players automove flag\n"
        "  -b            Set or unset only the black players automove flag\n"
        ) {

  bool set = true;
  enum { WHITE, BLACK, BOTH } color = BOTH;

  optind = 0;
  while (true) {
    int c = getopt(argc, argv, "dwb");
    switch (c) {
    case -1:
      goto end_of_parsing;
    case '?':
      break;
    case 'd':
      set = false;
      break;
    case 'w':
      color = WHITE;
      break;
    case 'b':
      color = BLACK;
      break;
    }
  }

 end_of_parsing:
  if (color == WHITE || color == BOTH)
    global_options.white_automove = set;

  if (color == BLACK || color == BOTH)
    global_options.black_automove = set;

  make_automove();
  return true;
}

command(playai,
        "Make a random move generated by AI",
        "Usage: playai\n"
        "\n"
        "Make a random move generated by AI.\n"
        ) {

  // Check if the game ended.
  if (game_state.status != NORMAL) {
    io_error();
    pp_f("error: could not play any moves, game ended\n");
    return false;
  }

  // Select one of the moves generated by the AI and make move.
  io_info();
  pp_f("playing... ");
  move_t best_moves[256];
  size_t best_moves_length;
  eval_t eval = evaluate(&game_state, &game_history, global_options.ai_depth, best_moves, &best_moves_length);

  do_move(&game_state, &game_history, best_moves[rand() % best_moves_length]);
  io_info();
  pp_f("done\n");
  pp_eval(eval, game_state.board, &game_history);

  make_automove();
  return true;
}

command(evaluate,
        "Evaluate the board and print evaluation information",
        "Usage: evaluate [OPTION]...\n"
        "\n"
        "Evaluate the board and print evaluation score.\n"
        "\n"
        "  -r                 Print one of the generated moves\n"
        "  -l                 Print the list of generated moves\n"
        ) {

  enum { LIST, RANDOM_MOVE, EVAL_TEXT } evaluation_type = EVAL_TEXT;

  optind = 0;
  while (true) {
    int c = getopt(argc, argv, "rl");
    switch (c) {
    case '?':
      return false;
      break;
    case 'r':
      evaluation_type = RANDOM_MOVE;
      break;
    case 'l':
      evaluation_type = LIST;
      break;
    case -1:
      goto end_of_parsing;
    }
  }

 end_of_parsing:
  // Check if the game ended.
  if (game_state.status != NORMAL) {
    io_error();
    pp_f("error: game ended\n");
    return true;
  }

  // Print the calculated evaluation of the AI.
  io_info();
  pp_f("evaluating... ");
  move_t best_moves[256];
  size_t best_moves_length;
  eval_t eval;

#ifdef MEASURE_EVAL_TIME
  {
    clock_t start = clock();
#endif

    eval = evaluate(&game_state, &game_history, global_options.ai_depth, best_moves, &best_moves_length);
    io_info();
    pp_f("done\n");

#ifdef MEASURE_EVAL_TIME
    pp_f("info: took %dms\n", (clock() - start) / (CLOCKS_PER_SEC / 1000));
  }
#endif

#ifdef MEASURE_EVAL_COUNT
  io_info();
  pp_f("info: called _evaluate %d times.\n",
       get_evaluate_count());
  pp_f("info: cut %d branches.\n",
       get_ab_branch_cut_count());
  pp_f("info: found %d (%d %%) different game ends.\n",
       get_game_end_count(),
       get_game_end_count() * 100 / get_evaluate_count());
  pp_f("info: found total %d (%d %%) leaves.\n",
       get_leaf_count(),
       get_leaf_count() * 100 / get_evaluate_count());

#ifdef MM_OPT_MEMOIZATION
  pp_f("info: in total, used %d (%d %%) transposition tables entries.\n",
       get_tt_saved_count(),
       get_tt_saved_count() * 100 / AI_HASHMAP_SIZE);
  pp_f("info: remembered %d (%d %% per call, %d %% per entry) times.\n",
       get_tt_remember_count(),
       get_tt_remember_count() * 100 / get_evaluate_count(),
       get_tt_remember_count() * 100 / get_tt_saved_count());
  pp_f("info: overwritten the same board %u (%u %%) times.\n",
       get_tt_overwritten_count(),
       get_tt_overwritten_count() * 100 / get_tt_saved_count());
  pp_f("info: rewritten a different board %u (%u %%) times.\n",
       get_tt_rewritten_count(),
       get_tt_rewritten_count() * 100 / get_tt_saved_count());
#endif


#endif

  switch (evaluation_type) {
  case LIST:
    io_basic();
    pp_moves(best_moves, best_moves_length);
    pp_f("\n");
    break;
  case RANDOM_MOVE:
    io_basic();
    pp_move(best_moves[rand() % best_moves_length]);
    pp_f("\n");
    break;
  case EVAL_TEXT:
    io_basic();
    pp_eval(eval, game_state.board, &game_history);
    break;
  }

  return true;
}

command(placeat,
        "Place a piece at a position",
        "Usage: placeat POS PIECE\n"
        "\n"
        "Place PIECE at POS.\n"
        ) {

  if (argc != 3) {
    io_error();
    pp_f("error: placeat requires exactly 2 arguments.\n");
    return false;
  }

  pos_t pos;
  char piece = argv[2][0];
  if (!string_to_position(argv[1], &pos)) {
    io_error();
    pp_f("error: invalid position '%s'\n", argv[1]);
    return false;
  }

  place_piece(&game_state, &game_history, pos, piece);
  return true;
}

command(removeat,
        "Remove the piece at a position",
        "Usage: removeat POS\n"
        "\n"
        "Remove the piece at POS.\n"
        ) {

  if (argc != 2) {
    io_error();
    pp_f("placeat requires exactly 1 arguments.\n");
    return false;
  }

  pos_t pos;
  if (!string_to_position(argv[1], &pos)) {
    io_error();
    pp_f("error: invalid position '%s'\n", argv[1]);
    return false;
  }

  remove_piece(&game_state, &game_history, pos);
  return true;
}

command(aidepth,
        "Set the search depth of the AI",
        "Usage: aidepth [DEPTH]\n"
        "\n"
        "Set the search depth of the AI to DEPTH if DEPTH is given. Otherwise print.\n"
        ) {

  switch (argc) {
  case 1:
    io_basic();
    pp_f("%zu\n", global_options.ai_depth);
    return true;
  case 2:
    global_options.ai_depth = atoi(argv[1]);
    return true;
  default:
    io_error();
    pp_f("command 'aidepth' expects 0 or 1 argument.\n");
    return false;
  }
}

static inline size_t count_branches(size_t depth) {
  if (!depth) return 1;

  // Check if reached a end of game node.
  if (game_state.status != NORMAL) {
    return 1;
  }

  // Count all of the nodes.
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

command(test,
        "Run a test command",
        "Usage: test [OPTION]...\n"
        "\n"
        "Run a test command.\n"
        "\n"
        "  -l [DEPTH]                   Count the number of reachable leaves in DEPTH ply.\n"
        ) {

  size_t depth = 0;
  enum { LEAF_COUNT } test = LEAF_COUNT;

  optind = 0;
  while (true) {
    int c = getopt(argc, argv, "l:");
    switch (c) {
    case '?':
      return false;
    case 'l':
      depth = atoi(optarg);
      test = LEAF_COUNT;
      break;
    case -1:
      switch (test) {
      case LEAF_COUNT:
        io_basic();
        pp_f("%zu\n", count_branches(depth));
        return true;
      }
    }
  }
}

typedef struct {
  const char* name;
  bool (* function) (int, char**);
  const char* simple_description;
  const char* full_description;
} command_entry_t;

command_entry_t* command_entries;

command(help,
        "Get information about commands",
        "Usage: help [COMMAND]\n"
        "\n"
        "Print this message or get information about COMMAND.\n"
        ) {

  io_info();
  if (argc == 1) {
    pp_f("For more information on a command, use 'help COMMAND'\n\n");
    for (size_t i=0; command_entries[i].function != NULL; i++) {
      const int message_x = 16;
      command_entry_t command = command_entries[i];
      pp_f("  %s", command.name);
      for (int i=0; i<message_x - strlen(command.name); i++) {
        pp_f(" ");
      }
      pp_f("%s\n", command.simple_description);
    }
    pp_f("\n");
    return true;
  }

  // Check for the command name in the command entries.
  for (size_t command_id=0; command_entries[command_id].name != NULL; command_id++) {
    if (!strcmp(argv[1], command_entries[command_id].name)) {
      pp_f("\n%s\n", command_entries[command_id].full_description);
      return true;
    }
  }

  io_error();
  pp_f("error: unknown command '%s'\n", argv[1]);
  return false;
}


// -- Parsing command arguments --
bool is_whitespace(char c) { return c == ' ' || c == '\t' || c == '\n'; }

#define ARG_BUFFER_SIZE 512
#define ARGV_SIZE 32

int generate_argv(char* arg_buffer, char** argv) {
  // Get the command and parse it.
  char buffer[256];

  io_info();
  pp_f("> ");

  char* result = fgets(buffer, sizeof(buffer), stdin);

  if (global_options.interactive) {
    if (feof(stdin)) {
      pp_f("\n");
      exit(0);
    }
  } else {
    if (result != NULL) {
      pp_f("[filled from pipe] -- %s", result);
    }

    if (feof(stdin)) {
      pp_f("[end of pipe]\n");
      exit(0);
    }
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
      io_error();
      pp_f("error: argv overflow\n");
      return -1;
    }
    argv[argc++] = arg_buffer_ptr;

    while (quote ? c != quote : !is_whitespace(c)){
      if (c == '\0') {
        if (quote) {
          io_error();
          pp_f("error: unterminated quote\n");
          return -1;
        }

        break;
      }

      // Check for buffer overflow and move the character.
      if (arg_buffer_ptr > arg_buffer + ARG_BUFFER_SIZE) {
        io_error();
        pp_f("error: arg buffer overflow\n");
        return -1;
      }
      *arg_buffer_ptr++ = c;

      c = *buffer_ptr++;
    }

    // Check for buffer overflow and move the character.
    if (arg_buffer_ptr > arg_buffer + ARG_BUFFER_SIZE) {
      io_error();
      pp_f("error: arg buffer overflow\n");
      return -1;
    }
    *arg_buffer_ptr++ = '\0';
  }

  // After all of the buffers are filled, add a null ptr to the arg_ptr_buffer.
  if (argc > ARGV_SIZE) {
    io_error();
    pp_f("error: argv overflow\n");
    return -1;
  }

  argv[argc] = 0;
  return argc;
}


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

  global_options.ai_depth = 8;
  global_options.white_automove = false;
  global_options.black_automove = false;
}

#define command_entry(_name)                            \
  (command_entry_t) {                                   \
    .name = #_name,                                     \
    .function = command_##_name,                        \
    .simple_description = command_##_name##_simple,     \
    .full_description = command_##_name##_usage,        \
  }                                                     \

int main(int argc, char **argv){
  command_entry_t _commands[] = {
    command_entry(help),
    command_entry(show),
    command_entry(loadfen),
    command_entry(savefen),
    command_entry(makemove),
    command_entry(undomove),
    command_entry(automove),
    command_entry(status),
    command_entry(allmoves),
    command_entry(placeat),
    command_entry(removeat),
    command_entry(aidepth),
    command_entry(playai),
    command_entry(evaluate),
    command_entry(test),
    { NULL, NULL, NULL, NULL },
  };
  command_entries = _commands;

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

    char* argv[32];
    char arg_buffer[512];
    int argc = generate_argv(arg_buffer, argv);

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
