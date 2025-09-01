#include "pch.h"
#include "main.h"
#include "framebuffer.h"

const char HELPOPT[] ={'-','-','h','e','l','p'};

int main(int argn, char* argv[]) {
  parseArgs(argn, argv);
  framebuffer fb;
  fb.init("/dev/fb0");
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
