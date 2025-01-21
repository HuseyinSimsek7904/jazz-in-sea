#ifndef _IO_PP_H
#define _IO_PP_H

#include "commands/globals.h"

#include "ai/eval_t.h"
#include "board/board_t.h"
#include "board/piece_t.h"
#include "board/pos_t.h"
#include "move/move_t.h"

#include <stdarg.h>
#include <stdio.h>

static inline bool is_whitespace(char c) { return c == ' ' || c == '\t' || c == '\n'; }

char col_name(int);
char row_name(int);

piece_t char_to_piece(char);
bool string_to_position(const char*, pos_t*);
bool string_to_move(const char*, board_t, move_t*);

void fprint_position(FILE*, pos_t);
void fprint_move(FILE *, move_t);
void fprint_moves(FILE*, move_t *, size_t);

void fprint_board(FILE*, board_t, bool);
void fprint_islands(FILE *, board_state_t *, bool);

void fprint_eval(FILE *, eval_t, history_t *);

static inline void io_basic() { global_options.current_file = global_options.file_basic; }
static inline void io_error() { global_options.current_file = global_options.file_error; }
static inline void io_debug() { global_options.current_file = global_options.file_debug; }
static inline void io_info() { global_options.current_file = global_options.file_info; }

static inline void pp_f_va(const char* format, va_list args) {
  vfprintf(global_options.current_file, format, args);
}

static inline void pp_f(const char* format, ...) {
  va_list args;
  va_start(args, format);

  pp_f_va(format, args);
}

static inline void pp_board(board_t board, bool color) {
  fprint_board(global_options.current_file, board, color);
}

static inline void pp_move(move_t move) {
  fprint_move(global_options.current_file, move);
}

static inline void pp_moves(move_t* moves, size_t size) {
  fprint_moves(global_options.current_file, moves, size);
}

static inline void pp_islands(board_state_t* state, bool color) {
  fprint_islands(global_options.current_file, state, color);
}

static inline void pp_position(pos_t pos) {
  fprint_position(global_options.current_file, pos);
}

static inline void pp_eval(eval_t eval, board_t board, history_t* history) {
  fprint_eval(global_options.current_file, eval, history);
}

int generate_argv(char* buffer, char* arg_buffer, char** argv, const size_t arg_buffer_size, const size_t argv_size);

#endif
