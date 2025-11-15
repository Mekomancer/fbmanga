import fb;
import manga.dex;
import ui.tui;
#include "debug.h"
#include <curl/curl.h>
#include <clocale>
import config;
import types;
import std;
import png;
frame_buffer fb("/dev/fb0");
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
  if (!md.checkup()) {
    return -1;
  }
  std::vector<std::string> manga_ids = md.getMangaId();
  int manga_choice = tui.choose(manga_ids);
  std::vector<mangadex::chapter_info> chaps =
      md.getChapters(manga_ids[manga_choice]);
  std::vector<std::string> choices(chaps.size());
  std::transform(chaps.begin(), chaps.end(), choices.begin(),
                 [](mangadex::chapter_info info) { return info.desc; });
  int chapter_choice = tui.choose(choices);
  std::vector<std::string> img_urls = md.getImgUrls(chaps[chapter_choice].id);
  std::vector<png> pngs(img_urls.size());
  for (uint i = 0; i < pngs.size(); ++i) {
    md.downloadImg(img_urls[i], &pngs[i].in);
    pngs[i].init();
    pngs[i].parseHead();
    std::vector<rgb888> obuf;
    obuf.resize(pngs[i].ihdr.width * pngs[i].ihdr.height);
    pngs[i].decode();
    //    double factor = 479.0 / static_cast<double>(pngs[i].ihdr.width);
    for (uint line = 0; line < pngs[i].ihdr.height; line++) {
      display(pngs[i].image, line);
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
  }
  curl_global_cleanup();
  return 0;
}

int display(std::span<rgb888> image, int scroll) {
  for (uint col = 0; col < fb.vinfo.yres; col++) {
    for (uint row = 0; row < fb.vinfo.xres; row++) {
      if (0 < row + scroll && (row + scroll * 420 + col) < image.size()) {
        fb.setPixel(fb.vinfo.yres - 1 - col, row,
                    image[(row + scroll) * 420 + col]);
      }
    }
  }
  return 0;
}
