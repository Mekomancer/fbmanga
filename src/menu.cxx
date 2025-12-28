module;
#include <curses.h>
export module tui:menu;
import std;
import types;
struct MenuItem {
  virtual String text() = 0;
  virtual int action() = 0;
  int x;
  int y;
  Option<std::any> data;
};
struct Menu {
  String title;
  Vec<MenuItem *> items;
  void display();
  void display_title();
};
struct SubMenu : public MenuItem {
  String text() override { return menu.title; };
  int action() override {
    menu.display();
    return 0;
  };
  SubMenu(Menu submenu, int px, int py) {
    menu = submenu;
    x = px;
    y = py;
  }
  Menu menu;
};
struct ActionItem : public MenuItem {
  String action_name;
  String text() override { return action_name; };
  fn<int> functor;
  int action() override { return functor(); }
  ActionItem(StringView name, fn<int> act, int px, int py) {
    action_name = name;
    functor = act;
    x = px;
    y = py;
  }
};
export class MenuBuilder {
public:
  MenuBuilder &add_submenu(Menu submenu, int x = -1, int y = 0);
  MenuBuilder &add_action(StringView name, fn<int> action, int x = -1,
                          int y = 0);
  MenuBuilder &add_backbutton();
  MenuBuilder &add_input_field(StringView name, int len, int x = -1, int y = 0);
  MenuBuilder &add_results_menu(StringView name, StringView data_field,
                                fn<int, StringView> func, int x = -1,
                                int y = 0);
  Menu get() { return menu; };
  MenuBuilder &title(StringView title) {
    menu.title = title;
    curline = 2;
    return *this;
  };

private:
  Menu menu;
  int curline;
};

void Menu::display() {
  clear();
  refresh();
  display_title();
  int cur = 0;
  while (true) {
    for (int i = 0; i < items.size(); ++i) {
      if (cur == i) {
        attron(A_REVERSE);
      }
      mvaddstr(items[i]->x, items[i]->y, items[i]->text().c_str());
      if (cur == i) {
        attroff(A_REVERSE);
      };
    }
    int chr = getch();
    if (chr == '\n') {
      if (items[cur]->action() != 0) {
        break;
      }
      clear();
      display_title();
      refresh();
    } else if (chr == KEY_UP) {
      cur = --cur < 0 ? items.size() - 1 : cur;
    } else if (chr == KEY_DOWN) {
      cur = ++cur >= items.size() ? 0 : cur;
    }
  }
  refresh();
};

void Menu::display_title() { mvaddstr(0, 0, title.c_str()); };

MenuBuilder &MenuBuilder::add_submenu(Menu submenu, int x, int y) {
  if (x != -1) {
    curline = x;
  }
  menu.items.push_back(new SubMenu(submenu, curline++, y));
  return *this;
}
MenuBuilder &MenuBuilder::add_action(StringView name, fn<int> act, int x,
                                     int y) {
  if (x != -1) {
    curline = x;
  }
  menu.items.push_back(new ActionItem(name, act, curline++, y));
  return *this;
}

MenuBuilder &MenuBuilder::add_backbutton() {
  menu.items.push_back(
      new ActionItem("Back", []() -> int { return -1; }, 25, 0));
  return *this;
}

MenuBuilder &MenuBuilder::add_input_field(StringView name, int width, int x,
                                          int y) {

};
