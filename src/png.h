#include "util.h"

class png{
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
    rgb888 background;
    int hist(int len);
    int phys(int len);
    int splt(int len);
    int exif(int len);
    int time(int len);
    int trns(int len);
    void notImplYet(int len);
    std::map<std::string, std::array<char,4>> chunk_type{
      {"IHDR", {0x49,0x48,0x44,0x52}},
      {"PLTE", {0x50,0x4C,0x54,0x45}},
      {"IDAT", {0x49,0x44,0x41,0x54}},
      {"IEND", {0x49,0x45,0x4E,0x44}},
      {"tRNS", {0x74,0x52,0x4E,0x52}},
      {"cHRM", {0x63,0x48,0x52,0x4D}},
      {"gAMA", {0x67,0x41,0x4D,0x41}},
      {"iCCP", {0x69,0x43,0x43,0x50}},
      {"sBIT", {0x73,0x42,0x49,0x54}},
      {"sRGB", {0x73,0x52,0x47,0x42}},
      {"cICP", {0x63,0x49,0x43,0x50}},
      {"mDCV", {0x6D,0x44,0x43,0x56}},
      {"cLLI", {0x63,0x4C,0x4C,0x49}},
      {"tEXt", {0x74,0x45,0x58,0x74}},
      {"zTXt", {0x7A,0x54,0x58,0x74}},
      {"iTXt", {0x69,0x54,0x58,0x74}},
      {"bKGD", {0x62,0x4B,0x47,0x44}},
      {"hIST", {0x68,0x49,0x53,0x54}},
      {"pHYs", {0x70,0x48,0x59,0x73}},
      {"sPLT", {0x73,0x50,0x4C,0x54}},
      {"eXIf", {0x65,0x58,0x49,0x66}},
      {"tIME", {0x74,0x49,0x4D,0x45}},
       //this was all hand typed in, no copy paste, so maybe innacurate...
    };
    static constexpr std::array<uint8_t,8> signature{0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    std::vector<rgb888> palette;
    bool validate_ihdr() noexcept;
    uint32_t checksum;
    template<typename byte>
    void crc32(char *data, int len);
    bool checkCRC();
    int decodeImageData(uint32_t length);
    int filterline(uint8_t *buf, int length);
    std::vector<uint8_t> curline;
    std::vector<uint8_t> prevline;
    int bpp;
    bool validDepthColor();
    bool tainted;
    int writeLine();
    int scanline_mem;
    template<typename byte>
    int putData(byte *buffer, size_t num_bytes = 1);
  public:
    struct ihdr_t{
      uint32_t width;
      uint32_t height;
      uint8_t bit_depth;
      uint8_t color_type;
      uint8_t compression_method;
      uint8_t filter_method;
      uint8_t interlace_method;
    } ihdr;
    uint64_t image_size;
    ring_buf<std::byte> in;
    char *next_out;
    size_t avail_out;
    int init();
    int parseHead();
    int parsePalette(uint32_t length);
    int decode();
};
