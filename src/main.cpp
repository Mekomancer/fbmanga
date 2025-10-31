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
  std::vector<char> ibuf;
  ibuf.resize(stats.st_size);
  read(png_fd,ibuf.data(),stats.st_size);
  png pngs[1];
  int i = 0;
  pngs[i].next_in = ibuf.data();
  pngs[i].avail_in = ibuf.size();
  pngs[i].init();
  pngs[i].parseHead();
  std::vector<color888> obuf;
  obuf.resize(pngs[i].ihdr.width*pngs[i].ihdr.height);
  pngs[i].next_out = reinterpret_cast<char*>(obuf.data());
  pngs[i].avail_out = obuf.size()*3;
  pngs[i].decode();
  double factor = 479.0/static_cast<double>(pngs[i].ihdr.width);  
  image img(static_cast<double>(pngs[i].ihdr.height)*factor);
  img.scale(factor, obuf, pngs[i].ihdr.width, pngs[i].ihdr.height);
  for(int i = 0; i < img.height; i++){
    img.display(i);
    switch(wgetch(stdscr)){
      case KEY_UP:
	i -= 40;
	break;
      case KEY_DOWN:
	i += 40;
	break;
    }
  };
  curl_global_cleanup();
  endwin();
  return 0;
}

int initialize(){
  fb.init();
  std::setlocale(LC_ALL, "");
  savetty();
  initscr(); cbreak(); noecho(); nodelay(stdscr,true); curs_set(0);
  keypad(stdscr,true);
  //for some reason, the settings don't apply until a read is called
  wgetch(stdscr);
  curl_global_init(CURL_GLOBAL_ALL);
  return 0;
};
