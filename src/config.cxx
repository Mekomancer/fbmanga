module;
#include <cstdio>
export module config;
import std;
import types;

export class configuration {
public:
  std::map<std::string, bool> logging{
      {"all", false}, {"location", false}, {"error", true}, {"warn", true}};
  std::string mangadex_api_url = "api.mangadex."
#ifndef NDEBUG
                                 "dev";
#else
                                 "org";
#endif
  std::FILE *log = stdout;
  void indexArgs(int argn, char *argv[]);
  int parseArgs();

private:
  std::vector<std::string> args;
};

export configuration conf;
void configuration::indexArgs(int argn, char *argv[]) {
  for (int i = 0; i < argn; i++) {
    args.push_back(argv[i]);
  }
  return;
};
void printHelp() { std::println("USAGE: fbmanga [OPTION]... [TITLE|FILE]..."); }

void printVersion() { std::println("FBManga v0.1"); }

int configuration::parseArgs() {
  int v_count = 0;
  for (uint32_t i = 0; i < args.size(); ++i) {
    std::string arg = args[i];
    if (arg == "-h" || arg == "--help") {
      printHelp();
      std::exit(0);
    } else if (arg == "--version") {
      printVersion();
      std::exit(0);
    } else if (arg == "-v") {
      ++v_count;
      if (v_count > 0) {
        logging["all"] = true;
      }
      if (v_count > 1) {
        logging["location"] = true;
      }
    } else if (arg == "-f") {
      if (i + 1 < args.size()) {
        log = std::fopen(args[i + 1].c_str(), "w+");
        continue;
      }
      std::println("Filename expected");
    }
  }
  return 0;
}
