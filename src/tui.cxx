module;
#include <curses.h>
#include <sys/types.h>
export module ui.tui;
import std;
import debug;

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
    log("warn: tui already initialized",here());
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
    log("warn: tui not initialized, auto choosing {:}", here(), opts[0]);
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
    log("warn: tui not initialized so nothing to cleanup",here());
  }
}
