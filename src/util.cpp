#include "util.h"

size_t ring_buf::len() {
  if (is_wrapped()) {
    return end + (data.size() - (begin + 1));
  } else {
    return end - begin;
  }
}

void configuration::indexArgs(int argn, char *argv[]) {
  for (int i = 0; i < argn; i++) {
    args.push_back(argv[i]);
  }
  return;
};

int configuration::parseArgs() {
  for (int i = 0; i < args.size(); i++) {
    std::string arg = args[i];
    if (arg == "-h" || arg == "--help") {
      printHelp();
      std::exit(EXIT_SUCCESS); // asking for help is not a failure
    } else if (arg == "-v" || arg == "--version") {
      printVersion();
      std::exit(EXIT_SUCCESS);
    }
  }
  return 0;
}

void printHelp() { std::println("USAGE: fbmanga [OPTION]... [TITLE|FILE]..."); }

void printVersion() { std::println("FBManga v0.1"); }
