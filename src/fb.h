#include "util.h"
// see <linux/fb.h>
class frame_buffer {
public:
  bool constexpr inBounds(int row, int col) {
    return (0 <= row && row < vinfo.yres && 0 <= col && col < vinfo.xres);
  }
  fb_var_screeninfo vinfo;
  fb_fix_screeninfo finfo;
  void printInfo();
  void setPixel(int row, int col, rgb888 color) noexcept;
  rgb888 getPixel(int row, int col);
  frame_buffer(std::filesystem::path fb_device = "/dev/fb0");
  ~frame_buffer();

private:
  void *addr;
  int fd;
};
