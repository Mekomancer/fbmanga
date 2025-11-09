#include "manga.h"
#include "png.h"
#include "ui.h"
#include "util.h"
frame_buffer fb;
configuration conf;
text_user_interface tui;
void init() {
  std::setlocale(LC_ALL, "");
#ifdef NDEBUG
  tui.init();
#else
  dprf("Warn: tui not initialized, auto choosing\n");
#endif
  curl_global_init(CURL_GLOBAL_ALL);
  return;
};

void cleanup(){
#ifdef NDEBUG
    tui.cleanup()
#endif
}

std::string makefname(std::string_view manga, std::string_view chap, int img) {
  return format("{}.{}.{}.png", manga, chap, img);
}

int main(int argn, char *argv[]) {
  conf.indexArgs(argn, argv);
  conf.parseArgs();
  init();
  mangadex md;
  md.checkup();
  std::vector<std::string> manga_ids = md.getMangaId();
  int manga_choice = tui.choose(manga_ids);
  std::vector<std::string> chap_ids = md.getChapterIds(manga_ids[manga_choice]);
  int chapter_choice = tui.choose(chap_ids);
  std::vector<std::string> img_urls = md.getImgUrls(chap_ids[chapter_choice]);
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
