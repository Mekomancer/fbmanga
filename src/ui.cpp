#include "ui.h"
#include "util.h"

template <typename cat, int ct> void foo() { return (cat)ct; }

void text_user_interface::init() {
  if (initialized) {
    dprf("WARN: tui already initialized");
  }
  initscr();
  cbreak();
  noecho();
  nodelay(win, true);
  curs_set(CURSES_CURSOR_INVIS);
  keypad(win, true);
  refresh();
  initialized = true;
}

size_t text_user_interface::choose(std::span<std::string> opts) {
  if (!initialized) {
    dprf("Warn: tui not initialized, auto choosing {:}", opts[0]);
    return 0;
  } else {
  }
  return -1;
}

void text_user_interface::cleanup() {
  if (initialized) {
    endwin();
  } else {
    dprf("WARN: tui not initialized so nothing to cleanup");
  }
}
