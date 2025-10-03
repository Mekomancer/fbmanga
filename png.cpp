#include "png.h"
#include "view.h"

extern framebuffer fb;

int png_t::decode(datastream *png_data){
  return 0;
}

/*int png::validate(){
 *
  dprf("Validating PNG...");
  dprf("Checking for critcal chunks
  idat_index = std::find(index.begin(),index.end(),chunk_type::IDAT);
  dprf("IHDR first?...");
  dprf("{:}", index[0].type ==  chunk_type::IHDR);
  dprf("PLTE Before IDAT?", 
  PLTE.length % 3 == 0
*
  return 0;
};

*int png::createIndex(){
  dprf("Creating index...\n"); 
  char *cur = png_addr + 8;//the file sig is 8 bytes long
  png_chunk cur_chunk;
  int i=0;
  do{
  dprf("chunk {:d}",i);
  i++;
  cur_chunk.length = readBytes<uint32_t>(&cur, true);
  dprf("\tlength: {:d}",cur_chunk.length); 
  cur_chunk.type = static_cast<chunk_type>(readBytes<uint32_t>(&cur, false));
  dprf("\ttype: {:.4s}",reinterpret_cast<char*>(&cur_chunk.type));
  cur_chunk.data = cur; 
  cur+=cur_chunk.length;
  cur_chunk.crc = readBytes<uint32_t>(&cur, false);
  dprf("\tcrc: {:d}",cur_chunk.crc);
  index.push_back(cur_chunk);
  if(cur_chunk.type == chunk_type::PLTE){
    palette_index = i-1;
  }
  } while(cur-png_addr<stats.st_size);
  return 0;
}*/
int png_t::decodePalette(){
/*
  dprf("Loading palette...");
  if(chunk.type != chunk_type::PLTE){
    dprf("ERR: tried to load palette from a non-palette chunk {:.5?}", reinterpret_cast<char*>(&(chunk.type)));
    return -1;
  }
  if( chunk.length % 3 != 0 ){
    dprf("ERR: Invalid PLTE chunk, PLTE length ({:d} bytes) is not divisible by 3", chunk.length);
    return -1;
  }
  char *cur_byte = chunk.data;
  palette.resize(chunk.length/3);
  dprf("color entries: {:d}",chunk.length/3);
  for(int i=0; (i*3) < chunk.length; i++){
    uint8_t red,green,blue = 0;
    red = *cur_byte; cur_byte++;
    green = *cur_byte; cur_byte++;
    blue = *cur_byte; cur_byte++;
    palette[i] = rgb888topixel(red,green,blue);
    dprf("  {:3>d}: {:02X} {:02X} {:02X}",i,red,green,blue);
  };
#ifndef NDEBUG
  for(int i = chunk.length/3 - 1; i >=0; i--){ 
    for(int dx = 0; dx < 12; dx++){
      for(int dy = 0; dy < 12; dy++){
	fb[dx,12*(25+i-chunk.length/3)+dy] = palette[i];
      }
    }
  }
#endif*/
  return 0;
};

int png_t::decodeHeader(){
/*  dprf("Parsing header...");
  auto hdr = index[0];
  if(hdr.type != chunk_type::IHDR){
    dprf("ERR: First chunk is not IHDR, (got {:4?})",reinterpret_cast<char*>(&hdr.type));
  }
  char *cur = hdr.data;
  bool bad_header = false;
  width = readBytes<uint32_t>(&cur, true);
  dprf("Width: {:}, ", width);
  height = readBytes<uint32_t>(&cur, true);
  dprf("Height: {:}, ", height);
  bit_depth = readBytes<uint8_t>(&cur, false);
  dprf("Bit depth: {:}, ", bit_depth);
  color_type = static_cast<png_pixel_type>(readBytes<uint8_t>(&cur, false));
  dprf("Color type: {:}", to_string(color_type));
  compression_method = readBytes<uint8_t>(&cur, false);
  if(compression_method == 0){
    dprf("Compression method: 0: DEFLATE");
  } else {
    dprf("ERR: Unknown compression method {:}", compression_method);
    bad_header = true;
  }
  filter_method = readBytes<uint8_t>(&cur, false);
  if(filter_method == 0){
    dprf("Filter method: 0: Adaptive filtering with five basic filter types");
  } else {
    dprf("ERR: Unknown filter method {:}", filter_method);
    bad_header = true;
  }
  interlace_method = readBytes<uint8_t>(&cur, false);
  if(interlace_method == 0){
    dprf("Interlace method: 0: No interlace");
  }else if( interlace_method == 1){
    dprf("Interlace method: 1: Adam7 interlace");
  } else {
    dprf("ERR: Unknown filter method {:}", interlace_method);
    bad_header = true;
  }
  return bad_header?-1:0;
  */ return 0;
};

constexpr std::string colorTypeString(color_type_t val){ 
  switch (val){
   using enum color_type_t;
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
/*
constexpr std::string libdeto_string(enum libdeflate_result val){
	 The data would have decompressed to more than 'out_nbytes_avail'
	 * bytes.  
  switch (val){
   case LIBDEFLATE_SUCCESS:
     return "Decompression was successful.";
   case LIBDEFLATE_BAD_DATA:
     return "Decompression failed because the compressed data was invalid, corrupt, or otherwise unsupported.";
   case LIBDEFLATE_SHORT_OUTPUT:
     return "A NULL 'actual_out_nbytes_ret' was provided, but the data would have decompressed to fewer than 'out_nbytes_avail' bytes.";
   case LIBDEFLATE_INSUFFICIENT_SPACE:
     return "The data would have decompressed to more than 'out_nbytes_avail' bytes.";
   default:
     return "Unknown libdeflate decompression result";
  }
}*/
    

int png_t::decompress(){/*
  dprf("Extracting image data...");
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
img_m:em_len = height*(((width * bit_depth)/8) + 1);//byte aligned so no partial byte but add one for filter byte 
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




