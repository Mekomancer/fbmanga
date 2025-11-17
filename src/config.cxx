export module config;
import std;
import types;
export class configuration {
public:
  enum struct verboseness {
    err,
    warn,
    info,
    dump,
  } verbosity = verboseness::warn;
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

export configuration conf;
export typedef configuration::verboseness lvl;
void configuration::indexArgs(int argn, char *argv[]) {
  for (int i = 0; i < argn; i++) {
    args.push_back(argv[i]);
  }
  return;
};
void printHelp() { std::println("USAGE: fbmanga [OPTION]... [TITLE|FILE]..."); }

void printVersion() { std::println("FBManga v0.1"); }

int configuration::parseArgs() {
  for (uint32_t i = 0; i < args.size(); i++) {
    std::string arg = args[i];
    if (arg == "-h" || arg == "--help") {
      printHelp();
      std::exit(0);
    } else if (arg == "--version") {
      printVersion();
      std::exit(0);
    } else if (arg == "-v") {
      verbosity = verboseness::dump;
    }
  }
  return 0;
}
