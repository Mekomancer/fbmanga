module;
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <fcntl.h>
#include "debug.h"
export module fb;
import std;
import std.compat;
import types;
extern conf;
class frame_buffer {
public:
  bool constexpr inBounds(uint row, uint col) {
    return (0 <= row && row < vinfo.yres && 0 <= col && col < vinfo.xres);
  }
  fb_var_screeninfo vinfo;
  fb_fix_screeninfo finfo;
  void printInfo();
  void setPixel(int row, int col, rgb888 color) noexcept;
  rgb888 getPixel(int row, int col);
  frame_buffer() = delete;
  explicit frame_buffer(std::filesystem::path fb_device = "/dev/fb0");
  ~frame_buffer();

private:
  void *addr;
  int fd;
};
namespace fs = std::filesystem;
// this code was meant for a 565 screen, it may not work for a 888 screen
frame_buffer::frame_buffer(fs::path fb_dev) {
  fd = open(fb_dev.c_str(), O_RDWR);
  ioctl(fd, FBIOGET_FSCREENINFO, &finfo);
  ioctl(fd, FBIOGET_VSCREENINFO, &vinfo);
  addr = mmap(0, finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  info("frambuffer mmapped\n");
}

frame_buffer::~frame_buffer() { munmap(addr, finfo.smem_len); }

void frame_buffer::setPixel(int row, int col, rgb888 color) noexcept {
  reinterpret_cast<uint16_t *>(addr)[row * 320 + col] =
      (bitscale<uint16_t>(color.red, 8, vinfo.red.length) << vinfo.red.offset) |
      (bitscale<uint16_t>(color.grn, 8, vinfo.green.length)
       << vinfo.green.offset) |
      (bitscale<uint16_t>(color.blu, 8, vinfo.blue.length)
       << vinfo.blue.offset);
};

rgb888 frame_buffer::getPixel(int row, int col) {
  const uint16_t val = static_cast<uint16_t *>(addr)[row * 320 + col];
  return {bitscale<uint8_t>(val >> vinfo.red.offset &
                                ((1 << (vinfo.red.length)) - 1),
                            vinfo.red.length, 8),
          bitscale<uint8_t>(val >> vinfo.green.offset &
                                ((1 << (vinfo.green.length)) - 1),
                            vinfo.green.length, 8),
          bitscale<uint8_t>(val >> vinfo.blue.offset &
                                ((1 << (vinfo.blue.length)) - 1),
                            vinfo.blue.length, 8)};
};

void frame_buffer::printInfo() {
  std::print("{:-^42}\n", "Fixed Info");
  std::print("id: {:16}\n", finfo.id);
  std::print("smem_start: {:}\tsmem_len: {:}\n", finfo.smem_start,
             finfo.smem_len);
  std::print("type: {:}\ttype_aux: {:}\n", finfo.type, finfo.type_aux);
  std::print("visual: {:}\n", finfo.visual);
  std::print("xpanstep: {:}\nypanstep: {:}\tywrapstep: {:}\n", finfo.xpanstep,
             finfo.ypanstep, finfo.ywrapstep);
  std::print("line_length: {:}\n", finfo.line_length);
  std::print("mmio_start: {:}\tmmio_len: {:}\n", finfo.mmio_start,
             finfo.mmio_len);
  std::print("accel: {:}\n", finfo.accel);
  std::print("capabilities: {:}\n", finfo.capabilities);
  std::print("{:-^42}\n", "Variable Info");
  std::print("xres        : {:}\tyres        : {:}\n", vinfo.xres, vinfo.yres);
  std::print("xres_virtual: {:}\tyres_virtual: {:}\n", vinfo.xres_virtual,
             vinfo.yres_virtual);
  std::print("xoffset     : {:}\tyoffset     : {:}\n", vinfo.xoffset,
             vinfo.yoffset);
  std::print("bits_per_pixel: {:}\tgrayscale: {:}\n", vinfo.bits_per_pixel,
             vinfo.grayscale);
  std::print("red::   offset: {:}\tlength: {:}\tmsb_right: {:}\n",
             vinfo.red.offset, vinfo.red.length, vinfo.red.msb_right);
  std::print("green:: offset: {:}\tlength: {:}\tmsb_right: {:}\n",
             vinfo.green.offset, vinfo.green.length, vinfo.green.msb_right);
  std::print("blue::  offset: {:}\tlength: {:}\tmsb_right: {:}\n",
             vinfo.blue.offset, vinfo.blue.length, vinfo.blue.msb_right);
  std::print("nonstd: {:}\tactivate: {:}\n", vinfo.nonstd, vinfo.activate);
  std::print("height: {:} mm\twidth: {:} mm\n", vinfo.height, vinfo.width);
  std::print("pixclock: {:} ps\n", vinfo.pixclock);
  std::print("rotate: {:}\n", vinfo.rotate);
  return;
}
