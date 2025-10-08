#include "config.h"

int parseArgs(int argn, char *argv[]){
  std::vector<std::string> args;
  for(int i = 0; i <argn; i++){
    args.push_back(argv[i]);
  }
  return 0;
};

int printHelp(int lvl){
  if(lvl < 0){
    std::println("Unknown option try");
  }
  std::println("USAGE: fbmanga [OPTION]... [TITLE|FILE]...");
  return 0;
}

int printVer(){
  std::print("FBmanga v0.1");
  return 0;
}
