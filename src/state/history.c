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

#include "state/history.h"
#include "move/move_t.h"
#include <sys/cdefs.h>

bool check_for_repetition(history_t *history, hash_t hash, size_t repetition) {
  int repetition_count = 0;

  // We only need to check 1 board each 4 boards.
  size_t i = history->size;
  while (i >= 4) {
    i -= 4;
    history_item_t item = history->history[i];

    if (item.hash == hash && ++repetition_count >= repetition)
      return true;
  }

  return false;
}
