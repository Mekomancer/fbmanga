constexpr const int CURSES_CURSOR_INVIS = 0;

class text_user_interface {
public:
  void init(); // call before using
  void cleanup();
  size_t choose(std::span<std::string> options);
  size_t choose(std::span<std::pair<std::string, std::string>> options);

private:
  bool initialized = false;
  WINDOW *win = stdscr;
};
