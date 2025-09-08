#include "pch.h"
#include "png.h"

int png::load(std::string filename){
  int fd = open(filename.c_str(),O_RDONLY);
  if (validate(fd)==-1){ return -1;}
  fstat(fd,&stats);
  addr = static_cast<char*>(mmap(0,stats.st_size,PROT_READ | PROT_WRITE,MAP_PRIVATE,fd,0)); 
  cur = addr;
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
  memcpy(&header,cur,sizeof(IHDR));
  header.width = std::byteswap<uint32_t>(header.width);
  header.height = std::byteswap<uint32_t>(header.height);
  printf("width:%d, height:%d, bit depth:%d, pixel type:%d\n",header.width,header.height,header.bit_depth,header.color_type);
  cur+= 4+4+chunkDataLength()+4;    
  printf("ihdr size:%d , length:%d\n",sizeof(IHDR),std::byteswap<uint32_t>(header.length));
  return 0;
}

int png::loadPalette(){
  uint32_t data_length = chunkDataLength();
  if (data_length%3!=0){
    return -1;
  };
  printf("loading palette chunk (chunk type: %c%c%c%c)\n",cur[4],cur[5],cur[6],cur[7]);
  palette.resize(data_length/3);
  for(char index = 0;index*3<data_length;index++){
    
    memcpy(&palette[index],cur+8+index*3,3);
    printf("index: %d, r:%d, g:%d, b:%d\n", index, palette[index].red, palette[index].green, palette[index].blue);

  }
  cur+= 4+4+chunkDataLength()+4;    
  return 0;
};


int png::skimUntilIDAT(){
  while(memcmp(cur+4,"IDAT",4)!=0){
  printf("skipping chunk of type %c%c%c%c\n",cur[4],cur[5],cur[6],cur[7]);
  fflush(0);
  cur+= 4+4+chunkDataLength()+4;    
}
  return 0;
};

int png::chunkDataLength(){
  uint32_t data_length;
  memcpy(&data_length,cur,sizeof(4));
  data_length = std::byteswap<uint32_t>(data_length);
  return data_length;
}


