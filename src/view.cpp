#include "view.h"
#include <linux/fb.h>

template <typename pixel>
int framebuffer<pixel>::init(){
  int fd = open("/dev/fb0",O_RDWR);
  fb_fix_screeninfo finfo;
  ioctl(fd,FBIOGET_FSCREENINFO,&finfo);
  addr = static_cast<pixel*>( mmap(0, finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
  close(fd);
  return 0;
}

