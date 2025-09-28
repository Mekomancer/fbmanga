#include "png.h"
#include "view.h"

extern framebuffer fb;
int png::load(std::string filename){
  std::println("loading png {:?}",filename);
  int fd = open(filename.c_str(),O_RDONLY);
  char buf[8];
  std::print("Checking file signature...");
  read(fd, buf,8);
  std::println("file sig: {:?}",buf);
  std::println("png  sig: {:?}",png_file_signature);
  int same = memcmp(buf,png_file_signature,8);
  if(same==0){
  std::print("file starts with the png file signature");
  }else{
    std::println("ERR: Not a png file");
  };
  if (same!= 0){ 
    return -1;
  }
  fstat(fd,&stats);
  addr = static_cast<char*>(mmap(0,stats.st_size,PROT_READ | PROT_WRITE,MAP_PRIVATE,fd,0)); 
  if (addr == MAP_FAILED){ return -1;}
  std::println("Done loading, closing fd");
  close(fd);
  std::println("fd closed");
  createIndex();
  loadPalette(index[palette_index]);
  if(validate()==-1){
    return -1;
  }
  return 0;
}

png::~png(){
    munmap(addr,stats.st_size);
}

png::png(){}

int png::validate(){
 /*
  std::println("Validating PNG...");
  std::println("Checking for critcal chunks
  idat_index = std::find(index.begin(),index.end(),chunk_type::IDAT);
  std::print("IHDR first?...");
  std::println("{:}", index[0].type ==  chunk_type::IHDR);
  std::print("PLTE Before IDAT?",
  PLTE.length % 3 == 0
*/
  return 0;
};

int png::createIndex(){
  std::println("Creating index..."); fflush(0);
  char *cur = addr + 8;//the file sig is 8 bytes long
  png_chunk cur_chunk;
  int i=0;
  do{
  std::print("chunk {:d}",i);
  i++;
  memcpy(&cur_chunk.length, cur,4);
  cur_chunk.length=std::byteswap<uint32_t>(cur_chunk.length);
  std::print("\tlength: {:d}",cur_chunk.length); fflush(0);
  cur+=4;
  memcpy(&cur_chunk.type, cur,4);
  std::print("\ttype: {:.4s}",reinterpret_cast<char*>(&cur_chunk.type));
  cur+=4;
  cur_chunk.data = cur; 
  cur+=cur_chunk.length;
  memcpy(&cur_chunk.crc, cur,4);
  std::println("\tcrc: {:d}",cur_chunk.crc);
  cur+=4;
  index.push_back(cur_chunk);
  if(cur_chunk.type == chunk_type::PLTE){
    palette_index = i-1;
    std::println("palette index == {:d}",palette_index);
  }
  } while(cur-addr<stats.st_size);
  return 0;
}

int png::loadPalette(png_chunk chunk){
  std::println("Loading palette");fflush(0);
  if(chunk.type != chunk_type::PLTE){
    std::println("ERR: tried to load palette from a non-palette chunk {:.5?}", reinterpret_cast<char*>(&(chunk.type)));
    return -1;
  }
  if( chunk.length % 3 != 0 ){
    std::println("ERR: Invalid PLTE chunk, PLTE length ({:d} bytes) is not divisible by 3", chunk.length);
    return -1;
  }
  char *cur_byte = chunk.data;
  palette.resize(chunk.length/3);
  std::println("color entries: {:d}",chunk.length/3);fflush(0);
  for(int i=0; (i*3) < chunk.length; i++){
    uint8_t red,green,blue = 0;
    red = *cur_byte; cur_byte++;
    green = *cur_byte; cur_byte++;
    blue = *cur_byte; cur_byte++;
    palette[i] = std::byteswap<uint16_t>(rgb888topixel(red,green,blue));
    std::println("  {:3>d}: {:02X} {:02X} {:02X}",i,red,green,blue);fflush(0);
  };
  for(int i = chunk.length/3 - 1; i >=0; i--){ 
  for(int dx = 0; dx < 12; dx++){
     for(int dy = 0; dy < 12; dy++){
      fb[dx,12*(25+i-chunk.length/3)+dy] = palette[i];
    }
  }
  }
  sleep(5);


  return -1;
};

  


