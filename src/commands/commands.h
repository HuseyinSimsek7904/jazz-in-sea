/*
This file is part of JazzInSea.

JazzInSea is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

JazzInSea is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with JazzInSea. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _COMMANDS_COMMANDS_H
#define _COMMANDS_COMMANDS_H

#include <stdbool.h>

#define command_declare(name)            \
  bool command_ ## name(int argc, char** argv)

typedef struct {
  const char* name;
  bool (* function) (int, char**);
  const char** simple_description;
  const char** full_description;
} command_entry_t;

command_declare(loadfen);
command_declare(savefen);
command_declare(show);
command_declare(makemove);
command_declare(undomove);
command_declare(status);
command_declare(allmoves);
command_declare(automove);
command_declare(playai);
command_declare(evaluate);
command_declare(placeat);
command_declare(removeat);
command_declare(aidepth);
command_declare(aitime);
command_declare(test);
command_declare(help);

#endif
