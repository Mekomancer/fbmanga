#include "main.h"
#include "manga.h"
#include "view.h"
#include "config.h"

frame_buffer fb;

int main(int argn, char* argv[]) {
  initialize();
//  parseArgs(argn, argv);
  mangadex_manga acchi_kocchi;
  acchi_kocchi.getMangaIdFromTitle("Acchi Kocchi");
  acchi_kocchi.downloadChapter(1);
  int png_fd = open(argv[1],O_RDWR);
  struct stat stats;
  fstat(png_fd,&stats);
  auto ibuf = new char[stats.st_size];
  read(png_fd,ibuf,stats.st_size);
  png pngs[1];
  pngs[0].next_in = ibuf;
  pngs[0].avail_in = stats.st_size;
  pngs[0].init();
  auto obuf = new char[pngs[0].image_size];
  pngs[0].next_out = obuf;
  pngs[0].avail_out = pngs[0].image_size;
  pngs[0].decode();
  curl_global_cleanup();
  delete[] ibuf;
  delete[] obuf;
  return 0;
}

int initialize(){
  fb.init();
  curl_global_init(CURL_GLOBAL_ALL);
  return 0;
};
