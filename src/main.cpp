import fb;
import manga.dex;
import ui.tui;
import std;
#include <clocale>
#include <curl/curl.h>
import config;
import types;
import std;
import png;
import debug;

frame_buffer fb("/dev/fb0");
text_user_interface tui;

void init() {
  std::setlocale(LC_ALL, "");
#ifdef NDEBUG
  tui.init();
#else
  warn("tui not initialized, auto choosing\n");
#endif
  curl_global_init(CURL_GLOBAL_ALL);
  return;
};

void cleanup(){
#ifdef NDEBUG
  tui.cleanup()
#endif
  curl_global_cleanup();
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
  std::vector<png_t> pngs(img_urls.size());
  for (uint i = 0; i < pngs.size(); ++i) {
    md.downloadImg(img_urls[i], &pngs[i].in);
    pngs[i].init();
    pngs[i].parseHead();
    std::vector<rgb888> obuf(pngs[i].image_size);
    pngs[i].decode();

    for (uint32_t line = 0; line < pngs[i].ihdr.height; line++) {
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
