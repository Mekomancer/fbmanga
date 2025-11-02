#include "main.h"
#include "manga.h"
#include "view.h"
#include "config.h"

frame_buffer fb;

int main(int argn, char* argv[]) {
  initialize();
  configuration cfg;
  cfg.indexArgs(argn, argv);
  mangadex md;
  md.init();
  std::string manga_id = md.getMangaId();
  std::vector<std::string> chaps = md.getChapterIds(manga_id);
  for(int i = 0; i<1;i++){
    md.downloadChapter(chaps[i]);
  }
  int png_fd = open(argv[1],O_RDWR);
  struct stat stats;
  fstat(png_fd,&stats);
  auto ibuf = new char[stats.st_size];
  read(png_fd,ibuf,stats.st_size);
  png pngs[1];
  pngs[0].next_in = ibuf;
  pngs[0].avail_in = stats.st_size;
  pngs[0].init();
  pngs[0].parseHead();
  auto obuf = new char[pngs[0].image_size*3];//output is 888 so *3
  pngs[0].next_out = obuf;
  pngs[0].avail_out = pngs[0].image_size * 3;
  pngs[0].decode();
  double factor = 479.0/static_cast<double>(pngs[0].ihdr.width);  
  image img(static_cast<double>(pngs[0].ihdr.height)*factor);
  scale(factor, reinterpret_cast<color888*>(obuf),pngs[0].ihdr.width,pngs[0].ihdr.height,&img);
  for(int i = 0; i < img.height; i++){
    img.display(i);
  };
  curl_global_cleanup();
  delete[] ibuf;
  delete[] obuf;
  return 0;
}

int initialize(){
  fb.init();
  std::setlocale(LC_ALL, "");
  initscr(); cbreak(); noecho();
  curl_global_init(CURL_GLOBAL_ALL);
  return 0;
};
