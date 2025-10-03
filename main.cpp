#include "main.h"
#include "mangadex.h"
#include "view.h"
#include "png.h"

const char HELPOPT[] ={'-','-','h','e','l','p'};
framebuffer fb;

int main(int argn, char* argv[]) {
  initialize();
  parseArgs(argn, argv);
  mangadex_manga acchi_kocchi;
  acchi_kocchi.getMangaIdFromTitle("Acchi Kocchi");
  filestream png;
  png.fd = open(argv[1],O_RDWR);
  curl_global_cleanup();
  return 0;
}

int initialize(){
  fb.init();
  curl_global_init(CURL_GLOBAL_ALL);
  return 0;
};



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
