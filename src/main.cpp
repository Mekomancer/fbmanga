#include "manga.h"
#include "view.h"
#include "config.h"
#include "png.h"

frame_buffer fb;
configuration conf;

void init(){
  std::setlocale(LC_ALL, "");
  initscr(); cbreak(); noecho(); nodelay(stdscr,true); curs_set(0);
  keypad(stdscr,true);
  refresh();
  curl_global_init(CURL_GLOBAL_ALL);
  return;
};

std::string makefname(std::string mangaid, std::string chapterid, int image){
  return format("{}.{}.{}.png",mangaid,chapterid,image);
}

int main(int argn, char* argv[]){
  init();
  conf.indexArgs(argn, argv);
  mangadex md;
  md.init();
  int cur_chap = 0;
  std::vector<std::string> manga_ids = md.getMangaId("Acchi Kocchi");
  std::vector<std::string> chap_ids = md.getChapterIds(manga_ids[0]);
  std::vector<std::string> img_urls = {"","",""};//md.getImgUrls(chap_ids[cur_chap]);
  std::vector<png> pngs(img_urls.size());
  std::vector<image> imgs(img_urls.size());  
  int png_fd = open(argv[1],O_RDWR);
  struct stat stats;
  fstat(png_fd,&stats);
  ring_buf<std::byte> ibuf;
  ibuf.resize(stats.st_size);
  std::vector<std::byte> buf(stats.st_size);
  read(png_fd,buf.data(),stats.st_size);
  ibuf.pop();
  for(std::byte data : buf){
    ibuf.append(data);
  }
  int i = 0;
  pngs[i].in = ibuf;
  pngs[i].init();
  pngs[i].parseHead();
  std::vector<rgb888> obuf;
  obuf.resize(pngs[i].ihdr.width*pngs[i].ihdr.height);
  pngs[i].next_out = reinterpret_cast<char*>(obuf.data());
  pngs[i].avail_out = obuf.size()*3;
  pngs[i].decode();
  double factor = 479.0/static_cast<double>(pngs[i].ihdr.width);  
  imgs[i].resize(static_cast<size_t>(pngs[i].ihdr.height*factor*pngs[i].ihdr.width*factor));
  imgs[i].scale(factor, obuf, pngs[i].ihdr.width, pngs[i].ihdr.height);
  for(int line = 0; line < imgs[i].height; line++){
    imgs[i].display(line);
    switch(getch()){
      case KEY_UP:
	line -= 40;
	break;
      case KEY_DOWN:
	line += 40;
	break;
    }
  };
  curl_global_cleanup();
  endwin();
  return 0;
}


