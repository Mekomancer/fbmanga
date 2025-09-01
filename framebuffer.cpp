#include "pch.h"
#include "framebuffer.h"

int framebuffer::init(std::string path){
  int fbfd = open(path.c_str(),O_RDWR);
  addr = static_cast<short*>(mmap(0, 320*480*2, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0));
  close(fbfd);
  return 0;
};

short& framebuffer::operator[](int x, int y){
  return addr[(479-x)*320+y];
};
