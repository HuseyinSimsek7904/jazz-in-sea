#include "ai/eval_t.h"
#include "board/status_t.h"
#include "commands/globals.h"
#include "io/pp.h"
#include "io/fen.h"
#include "move/make_move.h"
#include "move/generation.h"
#include "ai/evaluation.h"
#include "move/move_t.h"

#include <bits/types/siginfo_t.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

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
  pp_f("automove...\n");

  move_t best_moves[256];
  size_t best_moves_length;
  evaluate(&game_state, &game_history, global_options.ai_depth, global_options.ai_time, best_moves, &best_moves_length);
  do_move(&game_state, &game_history, best_moves[rand() % best_moves_length]);

  io_info();
  pp_f("done automove\n");
  pp_board(game_state.board, false);

  make_automove();
}

#define command_define(name, simple, full)              \
  const char* command_ ## name ## _simple = simple;     \
  const char* command_ ## name ## _full = full;         \
  bool command_ ## name(int argc, char** argv)          \

command_define(loadfen,
               "Load a board position using FEN",
               "Usage: loadfen FEN\n"
               "   or: loadfen PATH -f\n"
               "\n"
               "Load a board configuration from FEN string.\n"
               "\n"
               "  -f            Load FEN from PATH\n") {

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

command_define(savefen,
               "Print the board position FEN",
               "Usage: savefen [OPTION]...\n"
               "\n"
               "Print the current board configuration.\n"
               "\n"
               "  -f PATH       Save the FEN to PATH\n") {

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

command_define(show,
               "Print the current board position",
               "Usage: show [OPTION]...\n"
               "\n"
               "Print the current board position.\n"
               "\n"
               "  -h            Print the hash value of the board instead\n"
               "  -i            Print the island table instead\n") {
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

command_define(makemove,
               "Make a move",
               "Usage: makemove MOVE\n"
               "\n"
               "Try to make MOVE if it is a valid move.\n") {
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

command_define(undomove,
               "Undo the last move in history",
               "Usage: undomove\n"
               "\n"
               "Undo the last move in history, if there exists one.\n") {

  if (!game_history.size) {
    io_error();
    pp_f("error: no previous move\n");
    return false;
  }

  undo_last_move(&game_state, &game_history);
  return true;
}

command_define(status,
               "Print the current board status",
               "Usage: status\n"
               "\n"
               "Print the current board status.\n") {

  io_basic();
  pp_f("%s\n", board_status_text(game_state.status));
  return true;
}

command_define(allmoves,
               "Print the available moves on the current board",
               "Usage: allmoves\n"
               "\n"
               "Print the available moves on the current board.\n") {

  move_t moves[256];
  size_t length = generate_moves(&game_state, moves);

  io_basic();
  pp_moves(moves, length);
  pp_f("\n");
  return true;
}

command_define(automove,
               "Set or unset the automove flag",
               "Usage: automove [OPTION]..."
               "\n"
               "Set the automove flag for both players. When a players automove flag is on and it is their move to play, the AI automatically generates and plays a random move.\n"
               "\n"
               "  -d            Unset the flags instead of setting them\n"
               "  -w            Set or unset only the white players automove flag\n"
               "  -b            Set or unset only the black players automove flag\n") {
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

command_define(playai,
               "Make a random move generated by AI",
               "Usage: playai\n"
               "\n"
               "Make a random move generated by AI.\n") {

  // Check if the game ended.
  if (game_state.status != NORMAL) {
    io_error();
    pp_f("error: could not play any moves, game ended\n");
    return false;
  }

  // Select one of the moves generated by the AI and make move.
  io_info();
  pp_f("playing...\n");

  move_t best_moves[256];
  size_t best_moves_length;
  evaluate(&game_state, &game_history, global_options.ai_depth, global_options.ai_time, best_moves, &best_moves_length);
  do_move(&game_state, &game_history, best_moves[rand() % best_moves_length]);

  io_info();
  pp_f("done\n");

  make_automove();
  return true;
}

command_define(evaluate,
               "Evaluate the board and print evaluation information",
               "Usage: evaluate [OPTION]...\n"
               "\n"
               "Evaluate the board and print the best moves and the evaluation score.\n"
               "\n"
               "  -r            Select and print one of the best moves\n"
               "  -l            Print the list of generated moves only\n"
               "  -e            Print the evaluation score only\n"
               ) {

  enum { LIST, RANDOM_MOVE, EVAL_TEXT, FULL } evaluation_type = FULL;

  optind = 0;
  while (true) {
    int c = getopt(argc, argv, "erl");
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
    case 'e':
      evaluation_type = EVAL_TEXT;
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
  pp_f("evaluating...\n");

  move_t best_moves[256];
  size_t best_moves_length;
  eval_t eval = evaluate(&game_state, &game_history, global_options.ai_depth,
                         global_options.ai_time, best_moves, &best_moves_length);

  io_info();
  pp_f("evaluating done\n");

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
    pp_f("\n");
    break;
  case FULL:
    io_basic();
    pp_moves(best_moves, best_moves_length);
    pp_f(" -> ");
    pp_eval(eval, game_state.board, &game_history);
    pp_f("\n");
    break;
  }

  return true;
}

command_define(placeat,
               "Place a piece at a position",
               "Usage: placeat POS PIECE\n"
               "\n"
               "Place PIECE at POS.\n") {

  if (argc != 3) {
    io_error();
    pp_f("error: placeat requires exactly 2 arguments.\n");
    return false;
  }

  pos_t pos;
  if (!string_to_position(argv[1], &pos)) {
    io_error();
    pp_f("error: invalid position '%s'\n", argv[1]);
    return false;
  }

  piece_t piece = char_to_piece(argv[2][0]);
  if (!piece) {
    io_error();
    pp_f("error: invalid piece '%c'\n", argv[2][0]);
    return false;
  }

  place_piece(&game_state, &game_history, pos, piece);
  return true;
}

command_define(removeat,
               "Remove the piece at a position",
               "Usage: removeat POS\n"
               "\n"
               "Remove the piece at POS.\n") {

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

command_define(aidepth,
               "Set the maximum allowed search depth of the AI",
               "Usage: aidepth [DEPTH]\n"
               "\n"
               "Set the search depth of the AI to DEPTH if DEPTH is given. Otherwise print.\n") {

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

command_define(aitime,
               "Set the maximum allowed search time of the AI",
               "Usage: aidepth [TIME]\n"
               "\n"
               "Set the search depth of the AI to TIME if TIME is given. Otherwise print.\n"
               "TIME is in milliseconds.\n") {

  switch (argc) {
  case 1:
    io_basic();
    pp_f("%zu\n", global_options.ai_time.tv_sec * 1000 + global_options.ai_time.tv_nsec / 1000000);
    return true;
  case 2:
    {
      long milliseconds = atoi(argv[1]);
      global_options.ai_time.tv_sec = milliseconds / 1000;
      global_options.ai_time.tv_nsec = (milliseconds % 1000) * 1000000;
      return true;
    }
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

bool wait_for_child_resp(pid_t pid, char buffer[256], FILE* child_stdout) {
  fgets(buffer, 256, child_stdout);

  // This command does not accept very long outputs return error.
  if (strlen(buffer) == 255) {
    return true;
  }

  int status;
  waitpid(pid, &status, WNOHANG);

  return status && WIFEXITED(status);
}

command_define(test,
               "Run a test command",
               "Usage: test [OPTION]...\n"
               "\n"
               "Run a test command.\n"
               "\n"
               "  -l DEPTH      Count the number of reachable leaves in DEPTH ply.\n"
               "  -f EXEC       Play a game against another AI process with the same time and depth limits.\n ") {

  optind = 0;
  while (true) {
    int c = getopt(argc, argv, "l:f:");
    switch (c) {
    case '?':
      return false;

    case 'f':
      {
        int child_stdin_fd[2];
        int child_stdout_fd[2];
        int child_stderr_fd[2];

        // Create pipe for stdin, stdout and stderr.
        if (pipe(child_stdin_fd) < 0) {
          io_error();
          pp_f("error: could not create pipe\n");
          return false;
        }

        if (pipe(child_stdout_fd) < 0) {
          io_error();
          pp_f("error: could not create pipe\n");
          return false;
        }

        if (pipe(child_stderr_fd) < 0) {
          io_info();
          pp_f("error: could not create pipe\n");
          return false;
        }

        pid_t pid = fork();
        switch (pid) {
        case -1:
          io_error();
          pp_f("error: could not fork child\n");
          return false;

        case 0:
          {
            // Child reads from the command pipe and writes to the return pipe.
            close(child_stdin_fd[1]);
            close(child_stdout_fd[0]);
            close(child_stderr_fd[0]);

            // Duplicate the pipes into the standard streams.
            dup2(child_stdin_fd[0], fileno(stdin));
            dup2(child_stdout_fd[1], fileno(stdout));
            dup2(child_stderr_fd[1], fileno(stderr));

            // Create the command strings using sprintf.
            char fen_buffer[256];
            get_fen_string(fen_buffer, &game_state);
            char loadfen_command[512];
            sprintf(loadfen_command, "loadfen '%s'", fen_buffer);

            char aitime_command[512];
            sprintf(aitime_command, "aitime %ld", global_options.ai_time.tv_sec * 1000 + global_options.ai_time.tv_nsec / 1000000);

            char aidepth_command[512];
            sprintf(aidepth_command, "aidepth %zu", global_options.ai_depth);

            // Create the process.
            char* argv[] = { optarg, loadfen_command, aitime_command, aidepth_command, NULL };

            execv(optarg, argv);

            io_error();
            pp_f("error: execv returned\n");

            close(child_stdin_fd[0]);
            close(child_stdout_fd[1]);
            close(child_stderr_fd[1]);
            exit(1);
          }

        default:
          // Parent writes to the pipe and reads from the return pipe.
          close(child_stdin_fd[0]);
          close(child_stdout_fd[1]);
          close(child_stderr_fd[1]);

          FILE* child_stdin = fdopen(child_stdin_fd[1], "a");
          FILE* child_stdout = fdopen(child_stdout_fd[0], "r");
          FILE* child_stderr = fdopen(child_stderr_fd[0], "r");

          char buffer[256];

          while (true) {
            // Before making any move, ask the child what the status is.
            // If it is different than ours, there is a problem.
            io_info();
            pp_board(game_state.board, false);
            pp_f("%s to move\n", game_state.turn ? "white" : "black");

            fprintf(child_stdin, "status\n");
            fflush(child_stdin);

            if (wait_for_child_resp(pid, buffer, child_stdout)) break;

            // Hacky way to remove the newline character at the end of the buffer.
            buffer[strlen(buffer) - 1] = '\0';

            // If the status was different than expected report error.
            if (strcmp(board_status_text(game_state.status), buffer)) {
              io_error();
              pp_f("error: status from child does not match\n");
              break;
            }

            if (game_state.status != NORMAL) {
              io_info();
              pp_f("game ended\n");
              pp_f("%s\n", board_status_text(game_state.status));
              break;
            }

            move_t move;
            if (game_state.turn) {
              // If it is our turn to play, generate a random best move.
              move_t best_moves[256];
              size_t best_moves_length;
              evaluate(&game_state, &game_history, global_options.ai_depth, global_options.ai_time, best_moves, &best_moves_length);
              move = best_moves[rand() % best_moves_length];

            } else {
              // If it is the child's turn to play, ask for a move.
              fprintf(child_stdin, "evaluate -r\n");
              fflush(child_stdin);
              if (wait_for_child_resp(pid, buffer, child_stdout)) break;

              // Remove the newline character after the move.
              buffer[strlen(buffer) - 1] = '\0';

              // If the move was invalid report error.
              if (!string_to_move(buffer, game_state.board, &move)) {
                io_error();
                pp_f("error: invalid move from child, '%s'\n", buffer);
                break;
              }
            }

            io_info();
            pp_f("playing move: ");
            pp_move(move);
            pp_f("\n");

            // Make move and ask the child to make the move on their board as well.
            do_move(&game_state, &game_history, move);
            fprintf(child_stdin, "makemove ");
            fprint_move(child_stdin, move);
            fprintf(child_stdin, "\n");
            fflush(child_stdin);
          }

          int status;
          waitpid(pid, &status, WNOHANG);
          if (WIFEXITED(status)) {
            pp_f("process exited with exit code %u\n", WEXITSTATUS(status));
          } else {
            kill(pid, SIGKILL);
            pp_f("killed process\n");
          }

          char buffer_stderr[256];
          io_debug();
          pp_f("error output from process:\n");
          while (true) {
            if (!fgets(buffer_stderr, 256, child_stderr)) break;
            pp_f("%s", buffer_stderr);
          }
          pp_f("\n");

          fclose(child_stdin);
          fclose(child_stdout);
          fclose(child_stderr);
          return true;
        }
      }

    case 'l':
      io_basic();
      pp_f("%zu\n", count_branches(atoi(optarg)));
      return true;

    case -1:
      return true;
    }
  }
}

command_define(help,
               "Get information about commands",
               "Usage: help [COMMAND]\n"
               "\n"
               "Print this message or get information about COMMAND.\n") {

  io_basic();
  if (argc == 1) {
    pp_f("For more information on a command, use 'help COMMAND'\n\n");
    for (size_t i=0; command_entries[i].function != NULL; i++) {
      const int message_x = 16;
      command_entry_t command = command_entries[i];
      pp_f("  %s", command.name);
      for (int i=0; i<message_x - strlen(command.name); i++) {
        pp_f(" ");
      }
      pp_f("%s\n", *command.simple_description);
    }
    pp_f("\n");
    return true;
  }

  // Check for the command name in the command entries.
  for (size_t command_id=0; command_entries[command_id].name != NULL; command_id++) {
    if (!strcmp(argv[1], command_entries[command_id].name)) {
      pp_f("\n%s\n", *command_entries[command_id].full_description);
      return true;
    }
  }

  io_error();
  pp_f("error: unknown command '%s'\n", argv[1]);
  return false;
}

#define command_entry(_name)                                    \
  (command_entry_t) {                                           \
    .name = #_name,                                             \
      .function = command_ ## _name,                            \
      .simple_description = &command_ ## _name ## _simple,      \
      .full_description = &command_ ## _name ## _full,          \
      },                                                        \

command_entry_t command_entries[] = {
  command_entry(help)
  command_entry(show)
  command_entry(loadfen)
  command_entry(savefen)
  command_entry(makemove)
  command_entry(undomove)
  command_entry(automove)
  command_entry(status)
  command_entry(allmoves)
  command_entry(placeat)
  command_entry(removeat)
  command_entry(aidepth)
  command_entry(aitime)
  command_entry(playai)
  command_entry(evaluate)
  command_entry(test)
  { NULL, NULL, NULL, NULL, },
};
