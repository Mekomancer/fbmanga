#include "ui.h"

void text_user_interface::init() {
  initscr();
  cbreak();
  noecho();
  nodelay(win, true);
  curs_set(CURSES_CURSOR_INVIS);
  keypad(win, true);
  refresh();
}
