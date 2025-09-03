#include "pch.h"
#include "main.h"
#include "mangadex.h"

const char HELPOPT[] ={'-','-','h','e','l','p'};

int main(int argn, char* argv[]) {
  curl_global_init(CURL_GLOBAL_ALL);
  parseArgs(argn, argv);
  curl_global_cleanup();
  return 0;
}

int parseArgs(int argn, char* argv[]) {
  for(int narg=0;narg<argn;narg++) {
    if(argv[narg]==HELPOPT){
      printHelp();
    }
  }
  return 0;
}

int printHelp(){
  return -1;
}
