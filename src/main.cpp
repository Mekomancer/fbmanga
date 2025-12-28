import std;
import fb;
import manga.dex;
import gui;
import tui;
import config;
import net;
void cleanup() { tui.cleanup(); }

void init(int argn, char *argv[]) {
  std::atexit(cleanup);
  tui.init();
  conf.indexArgs(argn, argv);
  conf.parseArgs();
  initialize_curl();
  fb.init();
  return;
};

int main(int argn, char *argv[]) {
  init(argn, argv);
  tui.display();
  cleanup();
  return 0;
};
