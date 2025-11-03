const int CURSES_CURSOR_INVIS = 0;

class text_user_interface {
public:
  void init(); // call before using
  void cleanup();

private:
  WINDOW *win = stdscr;
};
