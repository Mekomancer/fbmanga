#include "main.h"
#include "manga.h"
#include "view.h"
#include "png.h"
#include "config.h"

framebuffer fb;

int main(int argn, char* argv[]) {
  initialize();
  parseArgs(argn, argv);
  mangadex_manga acchi_kocchi;
  acchi_kocchi.getMangaIdFromTitle("Acchi Kocchi");
  acchi_kocchi.downloadChapter(1);
 // int png_fd = open(argv[1],O_RDWR);
 // auto rfunc = [png_fd](char* buf, int cnt){return read(png_fd,buf,cnt);};
  //pngs[0].decode(rfunc);
  curl_global_cleanup();
  return 0;
}

int initialize(){
  fb.init();
  curl_global_init(CURL_GLOBAL_ALL);
  return 0;
};
