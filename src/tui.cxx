module;
#include <clocale>
#include <curses.h>
export module tui;
import std;
import types;
import :menu;

class TextUserInterface {
public:
  void init();
  void cleanup();
  void display() { main_menu.display(); };

private:
  Menu search = MenuBuilder()
                    .title("Search")
                    .add_input_field("Title", 42)
                    .add_results_menu("Go", "Title", title_search, 2, 45)
                    .add_backbutton()
                    .get();
  Menu main_menu = MenuBuilder()
                       .title("FBManga")
                       .add_submenu(search)
                       .add_action("Quit (ctrl-c)",
                                   []() -> int {
                                     std::exit(0);
                                     return 0;
                                   })
                       .get();
};
export TextUserInterface tui;

void TextUserInterface::init() {
  std::setlocale(LC_ALL, "");
  initscr();
  cbreak();
  noecho();
  nl();
  curs_set(0);
  keypad(stdscr, true);
  refresh();
}
void TextUserInterface::cleanup() { endwin(); }
