#include "pch.h"
#include "png.h"

int png::load(std::string filename){
  int fd = open(filename.c_str(),O_RDONLY);
  if (validate(fd)==-1){ return -1;}
  fstat(fd,&stats);
  addr = static_cast<char*>(mmap(0,stats.st_size,PROT_READ | PROT_WRITE,MAP_PRIVATE,fd,0)); 
  cur+=8;
  if (addr == MAP_FAILED){ return -1;}
  return 0;
}

png::~png(){
    munmap(addr,stats.st_size);
}

png::png(){}

int png::validate(int fd){
  std::array<uint8_t,8> buf;
  read(fd, &buf,8);
  assert(buf==png_file_signature);
  return buf==png_file_signature?0:-1;
};
int png::parseIHDR(){
  header=static_cast<IHDR*>(cur);
  return 0;
};

