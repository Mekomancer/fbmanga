#include "view.h"

int framebuffer::init(){
  int fd = open("/dev/fb0",O_RDWR);
  addr = static_cast<uint16_t*>( mmap(0, res*2/*2 bytes = 16 bpp*/,PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
  close(fd);
  return 0;
}

uint16_t& framebuffer::operator[](int x, int y){
  return addr[(479-x)*320+y];
}
