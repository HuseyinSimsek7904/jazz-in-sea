#ifndef _OPTIONS_H
#define _OPTIONS_H

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

#endif
