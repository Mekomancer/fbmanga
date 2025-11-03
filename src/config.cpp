#include "config.h"
#include "options.h"

void configuration::indexArgs(int argn, char *argv[]) {
  for (int i = 0; i < argn; i++) {
    args.push_back(argv[i]);
  }
  return;
};

int configuration::parseArgs() {
  for (int i = 0; i < args.size(); i++) {
  }
  return 0;
}

int printHelp(std::string nullarg) {
  std::println("USAGE: fbmanga [OPTION]... [TITLE|FILE]...");
  return 0;
}

int printVersion(std::string nullarg) {
  std::print("FBmanga v0.1");
  return 0;
}
