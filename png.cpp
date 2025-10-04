#include "png.h"
#include "view.h"

extern framebuffer fb;
static constexpr void bswap(auto *val){
  *val = std::byteswap(*val);
};

int png::decode(std::function<int(char*,int)> rfunc){
  dprf("Decoding png...\n");
  _read = rfunc;
  dprf("  Checking file signature...\n");
  std::array<char,8> file_sig;
  _read(file_sig.data(),8);
  dprf("    file sig: {:?} {:?} {:?} {:?} {:?} {:?} {:?} {:?}\n",file_sig[0],file_sig[1],file_sig[2],file_sig[3],file_sig[4],file_sig[5],file_sig[6],file_sig[7]);
  dprf("    png  sig: {:?} {:?} {:?} {:?} {:?} {:?} {:?} {:?}\n",png_sig[0],png_sig[1],png_sig[2],png_sig[3],png_sig[4],png_sig[5],png_sig[6],png_sig[7]);
  if(file_sig == png_sig){
    dprf("    File starts with the png file signature\n");
  }else{
    dprf("ERR: Not a png file\n");
    return -1;
  };
  decodeHeader();
  uint32_t crc;
  read(&crc);
  bool ended = false;
  while(!ended){
    uint32_t length;
    read(&length); bswap(&length);
    std::array<char,4> buf;
    _read(buf.data(),4);
    if(buf == chunk_type["PLTE"]){
      decodePalette(length);
    } else if(buf == chunk_type["IDAT"]){
      decodeImageData(length);
    } else if(buf == chunk_type["IEND"]){
      ended = true;
    }
    uint32_t crc;
    read(&crc);
  };
  return 0;
}

int png::decodePalette(uint32_t length){
  dprf("Decoding palette...");
  if( length % 3 != 0 ){
    dprf("ERR: Invalid PLTE chunk, PLTE length ({:d} bytes) is not divisible by 3", length);
    return -1;
  }
  palette.resize(length/3);
  dprf("color entries: {:d}\n",length/3);
  for(int i=0; (i*3) < length; i++){
    uint8_t red,green,blue = 0;
    read(&red);
    read(&green);
    read(&blue);
    palette[i] = rgb888topixel(red,green,blue);
    dprf("  {:3>d}: {:02X} {:02X} {:02X}\n",i,red,green,blue);
  };
#ifndef NDEBUG
  for(int i = length/3 - 1; i >=0; i--){ 
    for(int dx = 0; dx < 12; dx++){
      for(int dy = 0; dy < 12; dy++){
	fb[dx,12*(25+i-length/3)+dy] = palette[i];
      }
    }
  }
#endif
  return 0;
};

int png::decodeHeader(){
  dprf("Decoding header...\n");
  bool bad_header = false;
  uint32_t length = 0;
  read(&length); bswap(&length);
  if(length != 13){
    dprf("ERR: First chunk's length is not 13, (got {:})\n",length);
    return -1;
  };
  std::array<char,4> buf;
  _read(buf.data(),4);
  if(buf != chunk_type["IHDR"]){
    dprf("ERR: First chunk is not IHDR, (got {:?}{:?}{:?}{:?})\n",buf[0],buf[1],buf[2],buf[3]);
    return -1;
  }
  read(&stats.width); bswap(&stats.width);
  dprf("Width: {:}, ", stats.width);
  read(&stats.height); bswap(&stats.height);
  dprf("Height: {:}, ", stats.height);
  read(&stats.bit_depth);
  dprf("Bit depth: {:}, ", stats.bit_depth);
  read(&stats.color_type);
  dprf("Color type: {:}\n", colorTypeString(stats.color_type));
  read(&stats.compression_method);
  if(stats.compression_method == 0){
    dprf("Compression method: 0: DEFLATE\n");
  } else {
    dprf("ERR: Unknown compression method {:}\n", stats.compression_method);
    bad_header = true;
  }
  read(&stats.filter_method);
  if(stats.filter_method == 0){
    dprf("Filter method: 0: Adaptive filtering with five basic filter types\n");
  } else {
    dprf("ERR: Unknown filter method {:}\n", stats.filter_method);
    bad_header = true;
  }
  read(&stats.interlace_method);
  if(stats.interlace_method == 0){
    dprf("Interlace method: 0: No interlace\n");
  }else if( stats.interlace_method == 1){
    dprf("Interlace method: 1: Adam7 interlace\n");
  } else {
    dprf("ERR: Unknown filter method {:}\n", stats.interlace_method);
    bad_header = true;
  }
  return bad_header?-1:0;
};

constexpr std::string colorTypeString(color_type val){ 
  switch (val){
   using enum color_type;
    case greyscale:
      return "0: Greyscale";
    case truecolor:
      return "2: Truecolor";//there is no color type 1
    case palette:
      return "3: Indexed Color (palette)";
    case greyscale_alpha:
      return "4: Greyscale with alpha";
    case truecolor_alpha:
      return "6: Truecolor with alpha";//same here, no color type 5
    default:
      return std::format("\nERR: Invalid color type {:d}", static_cast<uint8_t>(val));
  }
}

std::map<std::string, std::array<char,4>> chunk_type{
  {"IHDR", {0x49,0x48,0x44,0x52}},
  {"PLTE", {0x50,0x4C,0x54,0x45}},
  {"IDAT", {0x49,0x44,0x41,0x54}},
  {"IEND", {0x49,0x45,0x4E,0x44}},
  {"tRNS", {0x74,0x52,0x4E,0x52}},
  //... more chunks types exist, but i'm too lazy
};
int png::decodeImageData(uint32_t length){
/*  dprf("Extracting image data...");
  size_t memlen = 0;
  for(png_chunk chunk : index){
    if(chunk.type == chunk_type::IDAT){
      memlen += chunk.length;
    }
  }
  dprf("Compressed size: {:d} bytes",memlen);
  char *compressed_data = reinterpret_cast<char*>(mmap(0,memlen,PROT_READ|PROT_WRITE,MAP_ANONYMOUS|MAP_SHARED,-1,0));
  if(compressed_data == MAP_FAILED){
    dprf("ERR: Faild to mmap memory for the compressed image data");
    return -1;
  };
  char *cur_addr = compressed_data;
  for(png_chunk chunk : index){
    if(chunk.type == chunk_type::IDAT){
      memcpy(cur_addr, chunk.data, chunk.length);
      cur_addr += chunk.length;
    }
  }
  dprf("Copied image data to input buffer for decompression");
  libdeflate_decompressor *decomp_ptr = libdeflate_alloc_decompressor();
  uint64_t img_mem_len;
  if(((width * bit_depth) % 8) == 0){
    img_mem_len = height*(((width * bit_depth)/8) + 1);//byte aligned so no partial byte but add one for filter byte 
  } else {
    img_mem_len = height * (((width*bit_depth)/8) + 2); //compensate for partial byte being truncated due to intger division and for filter byte
  }
  dprf("Calculated Image size:\t{:d} bytes",img_mem_len);
  img_addr = reinterpret_cast<char*>(mmap(0, img_mem_len, PROT_READ|PROT_WRITE,MAP_ANONYMOUS|MAP_SHARED,-1,0));
  size_t bytesout = 0;
  enum libdeflate_result result = libdeflate_zlib_decompress(decomp_ptr, compressed_data,memlen,img_addr,img_mem_len,&bytesout);
  dprf("libdeflate result: {:}", to_string(result));
  dprf("Actual Image size:\t{:d} bytes",bytesout);
  munmap( compressed_data, memlen);*/
  return 0;
}




