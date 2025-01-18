#include "state/history.h"
#include <sys/cdefs.h>

// Check if this move hash was repeated before REPETITION times.
bool check_for_repetition(history_t *history, hash_t hash, size_t repetition) {
  int repetition_count = 0;
  for (size_t i=0; i<history->size; i++) {
    history_item_t item = history->history[i];
    if (item.hash == hash) {
      if (++repetition_count >= repetition) {
        return true;
      }
    }
  }

  return false;
}
