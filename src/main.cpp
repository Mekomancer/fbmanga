import std;
import fb;
import manga.dex;
import ui.tui;
import ui.gui;
import config;
import types;
import png;
import util;
import debug;
import net;

constexpr int CURL_ERROR = -1;

void cleanup() {
  tui.cleanup();
}

void init(int argn, char *argv[]) {
  std::atexit(cleanup);
  conf.indexArgs(argn, argv);
  conf.parseArgs();
  tui.init();
  initialize_curl();
  frame_buffer fb("/dev/fb0");
  return;
};

std::string makefname(std::string_view manga, std::string_view chap, int img) {
  return format("{}.{}.{}.png", manga, chap, img);
}

int main(int argn, char *argv[]) {
  init(argn, argv);
  mangadex md;
  if(!md.checkup()) {
    return -1;
  }
  std::vector<std::string> manga_ids = md.getMangaId();
  int manga_choice = tui.choose(manga_ids);
  log("info: User picked choice no. {}, ({})\n", here(), manga_choice,
      manga_ids[manga_choice]);
  std::vector<mangadex::chapter_info> chaps =
      md.getChapters(manga_ids[manga_choice]);
  std::vector<std::string> choices(chaps.size());
  std::transform(chaps.begin(), chaps.end(), choices.begin(),
                 [](mangadex::chapter_info info) { return info.desc; });
  int chapter_choice = tui.choose(choices);
  std::vector<std::string> img_urls = md.getImgUrls(chaps[chapter_choice].id);
  std::vector<image_t> imgs(img_urls.size());
  for (int i = 0; i < img_urls.size(); ++i) {
    png_t png;
    png.init();
    std::print(".");
    if (md.downloadImg(img_urls[i], &png.in) < 0) {
      log("warn: failed to fetch image, skipping\n", here());
      continue;
    }
    std::print(".");
    png.parseHead();
    std::print(".");
    png.decode();
    std::print(".");
    imgs[i].resize(420, png.ihdr.height * 420 / png.ihdr.width);
    std::print(".");
    downscale_nearest(&png.image,&(imgs[i]));
    std::print(".\n");
  }
  for (uint32_t line = 0; line < imgs[0].yres(); line++) {
    Result<int,int> key;
    while(!(key = tui.get_key())){}
    
    log("dump: got key {:x}\n", here(), key.value());
    switch (key.value()) {
    case KEY_UP:
      line -= 40;
      break;
    case KEY_DOWN:
      line += 40;
      break;
    case 'q':
    case 'Q':
      std::exit(0);
    };
  }
  curl_global_cleanup();
  return 0;
};
