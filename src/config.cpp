#include "config.h"

int configuration::parseArgs(int argn, char *argv[]){
  std::vector<std::string> args;
  for(int i = 0; i <argn; i++){
    args.push_back(argv[i]);
  }
  return 0;
};

int _printHelp(std::string nullarg){
  std::println("USAGE: fbmanga [OPTION]... [TITLE|FILE]...");
  return 0;
}

int _printVersion(std::string nullarg){
  std::print("FBmanga v0.1");
  return 0;
}
