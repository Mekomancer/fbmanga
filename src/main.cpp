#include "main.h"
#include "config.h"
#include "manga.h"
#include "view.h"


int main(int argn, char *argv[]) {
frame_buffer fb;
  initialize();
/*  configuration cfg;
  cfg.indexArgs(argn, argv);
  mangadex md;
  md.init();
  std::string manga_id = md.getMangaId();
  std::vector<std::string> chaps = md.getChapterIds(manga_id);
  for (int i = 0; i < 1; i++) {
    md.downloadChapter(chaps[i]);
  }*/
  std::vector<png> pngs;
  int arg = 1;
  for (int n = 1; n < argn; n++) {
    png curpng;
    int png_fd = open(argv[n], O_RDWR);
    struct stat stats;
    fstat(png_fd, &stats);
    std::vector<char> ibuf(stats.st_size);
    read(png_fd, ibuf.data(), stats.st_size);
    curpng.next_in = ibuf.data();
    curpng.avail_in = stats.st_size;
    curpng.init();
    curpng.parseHead();
    std::vector<char> obuf(curpng.image_size * 3); // output is 888 so *3
    curpng.next_out = obuf.data();
    curpng.avail_out = obuf.size();
    curpng.decode();
    double factor = 479.0 / static_cast<double>(curpng.ihdr.width);
    image img(static_cast<double>(curpng.ihdr.height) * factor);
    scale(factor, reinterpret_cast<color888 *>(obuf.data()), curpng.ihdr.width,
          curpng.ihdr.height, &img);
    for (int scroll = 0; scroll + 320< img.height; scroll++) {
//      img.display(scroll);
    };
   // wgetch(stdscr);
    close(png_fd);
  }
  fb.free();
  endwin();
  //curl_global_cleanup();
  return 0;
}

int initialize() {
  std::setlocale(LC_ALL, "");
 /* initscr();
  cbreak();
  noecho();
  keypad(stdscr,true);
  curs_set(0);
  wrefresh(stdscr);*/
//  curl_global_init(CURL_GLOBAL_ALL);
  return 0;
};
