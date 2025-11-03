#include "util.h"

class frame_buffer {
private:
  void *addr;
  int fd;

public:
  bool constexpr inBounds(int row, int col) {
    return (0 <= row && row < vinfo.yres && 0 <= col && col < vinfo.xres);
  }
  fb_var_screeninfo vinfo;
  fb_fix_screeninfo finfo;
  void printInfo();
  int init(std::string_view fb_device = "/dev/fb0");
  int free();
  void setPixel(int row, int col, rgb888 color) noexcept;
  rgb888 getPixel(int row, int col);
};
