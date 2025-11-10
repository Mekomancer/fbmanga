#include "fb.h"
#include "util.h"

int display(double factor, std::span<rgb888> image, int w, int h, int scroll);
class png {
public:
  struct ihdr_t {
    uint32_t width;
    uint32_t height;
    uint8_t bit_depth;
    uint8_t color_type;
    uint8_t compression_method;
    uint8_t filter_method;
    uint8_t interlace_method;
  } ihdr;
  uint64_t image_size = 0;
  ring_buf in;
  std::vector<rgb888> image;
  int init();
  int parseHead();
  int parsePalette(uint32_t length);
  int decode();

private:
  int chrm(int len);
  int gama(int len);
  double gamma = 1.0;
  int iccp(int len);
  int sbit(int len);
  int srgb(int len);
  int cicp(int len);
  int mdcv(int len);
  int clli(int len);
  int text(int len);
  int ztxt(int len);
  int itxt(int len);
  int bkgd(int len);
  rgb888 background = {0, 0, 0};
  int hist(int len);
  int phys(int len);
  int splt(int len);
  int exif(int len);
  int time(int len);
  int trns(int len);
  void notImplYet(int len);
  std::map<std::string, uint32_t> chunk_type{
      {"IHDR", htop(0x49'48'44'52)}, {"PLTE", htop(0x50'4C'54'45)},
      {"IDAT", htop(0x49'44'41'54)}, {"IEND", htop(0x49'45'4E'44)},
      {"tRNS", htop(0x74'52'4E'52)}, {"cHRM", htop(0x63'48'52'4D)},
      {"gAMA", htop(0x67'41'4D'41)}, {"iCCP", htop(0x69'43'43'50)},
      {"sBIT", htop(0x73'42'49'54)}, {"sRGB", htop(0x73'52'47'42)},
      {"cICP", htop(0x63'49'43'50)}, {"mDCV", htop(0x6D'44'43'56)},
      {"cLLI", htop(0x63'4C'4C'49)}, {"tEXt", htop(0x74'45'58'74)},
      {"zTXt", htop(0x7A'54'58'74)}, {"iTXt", htop(0x69'54'58'74)},
      {"bKGD", htop(0x62'4B'47'44)}, {"hIST", htop(0x68'49'53'54)},
      {"pHYs", htop(0x70'48'59'73)}, {"sPLT", htop(0x73'50'4C'54)},
      {"eXIf", htop(0x65'58'49'66)}, {"tIME", htop(0x74'49'4D'45)},
      // this was all hand typed in, no copy paste, so may be innacurate...
  };
  static constexpr uint64_t signature = htop(0x89'50'4E'47'0D'0A'1A'0A);
  std::vector<rgb888> palette;
  bool valid_ihdr() noexcept;
  uint32_t checksum = 0;
  bool checkCRC(uint32_t length);
  int decodeImageData(uint32_t length);
  int filterline(const uint8_t *buf, int length);
  std::vector<uint8_t> curline;
  std::vector<uint8_t> prevline;
  int bpp = 0;
  bool validDepthColor();
  bool tainted = false;
  int writeLine();
  int scanline_mem = -1;
};
