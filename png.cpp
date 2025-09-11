#include "pch.h"
#include "png.h"

int png::load(std::string filename){
  printf("\nloading png %s",filename.c_str());
  int fd = open(filename.c_str(),O_RDONLY);
  if (validate(fd)==-1){ 
    return -1;
  }
  fstat(fd,&stats);
  addr = static_cast<char*>(mmap(0,stats.st_size,PROT_READ | PROT_WRITE,MAP_PRIVATE,fd,0)); 
  if (addr == MAP_FAILED){ return -1;}
  close(fd);
  loaded = true;
  printf("\nsuccessfully loaded, fd closed");
  return 0;
}

png::~png(){
    munmap(addr,stats.st_size);
}

png::png(){}

int png::validate(int fd){
  char buf[8];
  read(fd, buf,8);
  int same = memcmp(buf,png_file_signature.data(),8);
  if(same==0){
    printf("\nfile starts with the png file signature");
  }else{
    printf("\nERR: Not a png file");
  };
    printf("\n---------------------------------------");
    printf("\nfile sig: %.8s\n png sig: %.8s",buf,png_file_signature.data());
  return same==0?0:-1;
};

int png::createIndex(){
  if(loaded==false){
    printf("\nerr, png not loaded yet");
    return -1;
  };
  printf("\nCreating index..."); fflush(0);
  char *cur = addr + 8;
  png_chunk cur_chunk;
  int i=0;
  do{
  printf("\nchunk %u",i);
  i++;
  memcpy(&cur_chunk.length, cur,4);
  cur_chunk.length=std::byteswap<uint32_t>(cur_chunk.length);
  printf("\tlength: %u",cur_chunk.length); fflush(0);
  cur+=4;
  memcpy(&cur_chunk.type, cur,4);
  printf("\ttype: %.4s",reinterpret_cast<char*>(&cur_chunk.type));
  cur+=4;
  cur_chunk.data = cur; 
  cur+=cur_chunk.length;
  memcpy(&cur_chunk.crc, cur,4);
  printf("\tcrc: %u",cur_chunk.crc);
  cur+=4;
  index.push_back(cur_chunk);
  } while(cur-addr<stats.st_size);
  return 0;
}


