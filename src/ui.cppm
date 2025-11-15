module;
#include "debug.h"
#include <curses.h>
#include <sys/types.h>
import std;
import std.compat;

export module ui.tui;

constexpr const int CURSES_CURSOR_INVIS = 0;
export class text_user_interface {
public:
  void init(); // call before using
  void cleanup();
  size_t choose(std::span<std::string> options);

private:
  bool initialized = false;
  WINDOW *win = stdscr;
};

void printHelp();
void printVersion();

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
  for (std::string opt : opts) {
    std::print("", opt);
  }
  if (!initialized) {
#ifdef NDEBUG
    dprf("Warn: tui not initialized, auto choosing {:}", opts[0]);
#endif
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
