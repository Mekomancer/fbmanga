#include "config.h"
#include "manga.h"
#include "png.h"
#include "ui.h"
#include "view.h"

frame_buffer fb;
configuration conf;
text_user_interface tui;

void init() {
  std::setlocale(LC_ALL, "");
#ifdef NDEBUG
  tui.init()
#endif
      curl_global_init(CURL_GLOBAL_ALL);
  return;
};

std::string makefname(std::string_view mangaid, std::string_view chapterid,
                      int image) {
  return format("{}.{}.{}.png", mangaid, chapterid, image);
}

int main(int argn, char *argv[]) {
  init();
  conf.indexArgs(argn, argv);
  mangadex md;
  md.init();
  int cur_chap = 0;
  int i = 0;
  std::vector<std::string> manga_ids = md.getMangaId("Acchi Kocchi");
  std::vector<std::string> chap_ids = md.getChapterIds(manga_ids[0]);
  std::vector<std::string> img_urls = md.getImgUrls(chap_ids[cur_chap]);
  std::vector<png> pngs(img_urls.size());
  std::vector<image> imgs(img_urls.size());
  int png_fd = open(argv[1], O_RDWR);
  struct stat stats;
  fstat(png_fd, &stats);
  pngs[i].in.resize(stats.st_size);
  std::vector<std::byte> buf(stats.st_size);
  read(png_fd, buf.data(), stats.st_size);
  for (std::byte data : buf) {
    pngs[i].in.append(data);
  }
  pngs[i].in.pop();
  pngs[i].init();
  pngs[i].parseHead();
  std::vector<rgb888> obuf;
  obuf.resize(pngs[i].ihdr.width * pngs[i].ihdr.height);
  pngs[i].next_out = reinterpret_cast<char *>(obuf.data());
  pngs[i].avail_out = obuf.size() * 3;
  pngs[i].decode();
  double factor = 479.0 / static_cast<double>(pngs[i].ihdr.width);
  imgs[i].resize(static_cast<size_t>(pngs[i].ihdr.height * factor *
                                     pngs[i].ihdr.width * factor));
  imgs[i].scale(factor, obuf, pngs[i].ihdr.width, pngs[i].ihdr.height);
  for (int line = 0; line < imgs[i].height; line++) {
    imgs[i].display(line);
#ifdef NDEBUG
    switch (getch()) {
    case KEY_UP:
      line -= 40;
      break;
    case KEY_DOWN:
      line += 40;
      break;
    }
#endif
  };
  curl_global_cleanup();
  endwin();
  return 0;
}
