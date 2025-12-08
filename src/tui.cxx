module;
#include <clocale>
#include <curses.h>
export module ui.tui;
import std;
import types;
import debug;

constexpr const int CURSES_CURSOR_INVIS = 0;
constexpr const int CURSES_CURSOR_NORMAL = 0;

export class text_user_interface {
public:
  void init(); // call before using
  void cleanup();
  size_t choose(std::span<std::string> options);
  Result<int,int> get_key();
private:
  bool initialized = false;
  WINDOW *win = stdscr;
} tui;

Result<int,int> get_key(){
  int ret = getch();
  if(ret == ERR){
    return Err(ret);
  } else {
    return ret;
  }
}
  

void printHelp();
void printVersion();

void text_user_interface::init() {
  std::setlocale(LC_ALL, "");
  if (initialized) {
    log("warn: tui already initialized", here());
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
    log("warn: tui not initialized, auto choosing {:}", here(), opts[0]);
    return 0;
  } else {
    return 0;
  }
  return -1;
}

void text_user_interface::cleanup() {
  if (initialized) {
    curs_set(CURSES_CURSOR_NORMAL);
    nocbreak();
    echo();
    refresh();
    endwin();
    initialized = false;
  } else {
    log("warn: tui not initialized so nothing to cleanup", here());
  }
}
