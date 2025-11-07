#include "config.h"
#include "manga.h"
#include "png.h"
#include "ui.h"

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

void cleanup(){
#ifdef NDEBUG
    tui.cleanup()
#endif
}

std::string
    makefname(std::string_view mangaid, std::string_view chapterid, int image) {
  return format("{}.{}.{}.png", mangaid, chapterid, image);
}

int main(int argn, char *argv[]) {
  init();
  conf.indexArgs(argn, argv);
  mangadex md;
  md.init();
  int cur_chap = 0;
  int i = 0;
  md.checkup();
  std::vector<std::string> manga_ids = md.getMangaId();
  dprf("manga ids: ") for (std::string id : manga_ids) { dprf("{}", id); }
  std::vector<std::string> chap_ids = md.getChapterIds(manga_ids[0]);
  std::vector<std::string> img_urls = md.getImgUrls(chap_ids[cur_chap]);
  /* std::vector<png> pngs(img_urls.size());

  pngs[i].init();
  pngs[i].parseHead();
  std::vector<rgb888> obuf;
  obuf.resize(pngs[i].ihdr.width * pngs[i].ihdr.height);
  pngs[i].decode();
  double factor = 479.0 / static_cast<double>(pngs[i].ihdr.width);
  for (int line = 0; line < pngs[i].image.height; line++) {
    pngs[i].image.display(line);
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
  };*/
  curl_global_cleanup();
  return 0;
}
