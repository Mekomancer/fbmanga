#include <curses.h>
#include <sys/types.h>
import std;
import std.compat;
constexpr const int CURSES_CURSOR_INVIS = 0;
class configuration {
public:
  enum struct verboseness {
    silent,
    error,
    warn,
    info,
    dump,
  } vebosity = verboseness::warn;
  std::string mangadex_api_url = "api.mangadex."
#ifndef NDEBUG
                                 "dev";
#else
                                 "org";
#endif
  void indexArgs(int argn, char *argv[]);
  int parseArgs();

private:
  std::vector<std::string> args;
};

void configuration::indexArgs(int argn, char *argv[]) {
  for (int i = 0; i < argn; i++) {
    args.push_back(argv[i]);
  }
  return;
};
void printHelp() { std::println("USAGE: fbmanga [OPTION]... [TITLE|FILE]..."); }

void printVersion() { std::println("FBManga v0.1"); }

int configuration::parseArgs() {
  for (uint i = 0; i < args.size(); i++) {
    std::string arg = args[i];
    if (arg == "-h" || arg == "--help") {
      printHelp();
      std::exit(0);
    } else if (arg == "--version") {
      printVersion();
      std::exit(0);
    } else if (arg == "-v") {
      vebosity = verboseness::dump;
    }
  }
  return 0;
}


class text_user_interface {
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
