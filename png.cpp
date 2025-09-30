#include "png.h"
#include "view.h"

extern framebuffer fb;

template <typename data_type>
static data_type readBytes(char **ptr, bool do_byte_swap){
  data_type val = *reinterpret_cast<data_type*>(*ptr);
  *ptr += sizeof(data_type);
  if(do_byte_swap){ val = std::byteswap<data_type>(val); };
  return val;
}

png::png(){
  loaded = false;
  palette_index=-1;
}

int png::load(std::string filename){
  std::println("loading png {:?}",filename);
  int fd = open(filename.c_str(),O_RDONLY);
  char buf[8];
  std::println("Checking file signature...");
  read(fd, buf,8);
  std::println("file sig: {:?}",buf);
  std::println("png  sig: {:?}",png_file_signature);
  int same = memcmp(buf,png_file_signature,8);
  if(same==0){
    std::println("File starts with the png file signature");
  }else{
    std::println("ERR: Not a png file");
  };
  if (same!= 0){ 
    return -1;
  }
  fstat(fd,&stats);
  png_size = stats.st_size;
  png_addr = static_cast<char*>(mmap(0,png_size,PROT_READ | PROT_WRITE,MAP_PRIVATE,fd,0)); 
  if (png_addr == MAP_FAILED){ return -1;}
  std::print("Done loading, closing fd...");
  close(fd);
  std::println("Done");
  createIndex();
  parseHeader();
  if(color_type == png_pixel_type::palette){
    if(palette_index != -1 ){
      loadPalette(index[palette_index]);
    } else {
      std::print("ERR: PNG uses indexed color, but no palette/color index was found");
    }
  }
  if(validate()==-1){
    return -1;
  }
  getData();
  munmap(png_addr,png_size);
  png_addr = nullptr;
  return 0;
}

png::~png(){
  if(img_addr != nullptr){
    munmap(img_addr, img_mem_len);
  }
  if(png_addr != nullptr){
    munmap(png_addr, png_size);
  }
    
}

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
  char *cur = png_addr + 8;//the file sig is 8 bytes long
  png_chunk cur_chunk;
  int i=0;
  do{
  std::print("chunk {:d}",i);
  i++;
  cur_chunk.length = readBytes<uint32_t>(&cur, true);
  std::print("\tlength: {:d}",cur_chunk.length); fflush(0);
  cur_chunk.type = static_cast<chunk_type>(readBytes<uint32_t>(&cur, false));
  std::print("\ttype: {:.4s}",reinterpret_cast<char*>(&cur_chunk.type));
  cur_chunk.data = cur; 
  cur+=cur_chunk.length;
  cur_chunk.crc = readBytes<uint32_t>(&cur, false);
  std::println("\tcrc: {:d}",cur_chunk.crc);
  index.push_back(cur_chunk);
  if(cur_chunk.type == chunk_type::PLTE){
    palette_index = i-1;
    std::println("palette index == {:d}",palette_index);
  }
  } while(cur-png_addr<stats.st_size);
  return 0;
}

int png::loadPalette(png_chunk chunk){
  std::println("Loading palette...");fflush(0);
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
  std::println("color entries: {:d}",chunk.length/3);
  for(int i=0; (i*3) < chunk.length; i++){
    uint8_t red,green,blue = 0;
    red = *cur_byte; cur_byte++;
    green = *cur_byte; cur_byte++;
    blue = *cur_byte; cur_byte++;
    palette[i] = rgb888topixel(red,green,blue);
    std::println("  {:3>d}: {:02X} {:02X} {:02X}",i,red,green,blue);
  };
#ifndef NDEBUG
  for(int i = chunk.length/3 - 1; i >=0; i--){ 
    for(int dx = 0; dx < 12; dx++){
      for(int dy = 0; dy < 12; dy++){
	fb[dx,12*(25+i-chunk.length/3)+dy] = palette[i];
      }
    }
  }
#endif
  return 0;
};

int png::parseHeader(){
  std::println("Parsing header...");
  auto hdr = index[0];
  if(hdr.type != chunk_type::IHDR){
    std::println("ERR: First chunk is not IHDR, (got {:4?})",reinterpret_cast<char*>(&hdr.type));
  }
  char *cur = hdr.data;
  bool bad_header = false;
  width = readBytes<uint32_t>(&cur, true);
  std::print("Width: {:}, ", width);
  height = readBytes<uint32_t>(&cur, true);
  std::print("Height: {:}, ", height);
  bit_depth = readBytes<uint8_t>(&cur, false);
  std::print("Bit depth: {:}, ", bit_depth);
  color_type = static_cast<png_pixel_type>(readBytes<uint8_t>(&cur, false));
  std::println("Color type: {:}", to_string(color_type));
  compression_method = readBytes<uint8_t>(&cur, false);
  if(compression_method == 0){
    std::println("Compression method: 0: DEFLATE");
  } else {
    std::println("ERR: Unknown compression method {:}", compression_method);
    bad_header = true;
  }
  filter_method = readBytes<uint8_t>(&cur, false);
  if(filter_method == 0){
    std::println("Filter method: 0: Adaptive filtering with five basic filter types");
  } else {
    std::println("ERR: Unknown filter method {:}", filter_method);
    bad_header = true;
  }
  interlace_method = readBytes<uint8_t>(&cur, false);
  if(interlace_method == 0){
    std::println("Interlace method: 0: No interlace");
  }else if( interlace_method == 1){
    std::println("Interlace method: 1: Adam7 interlace");
  } else {
    std::println("ERR: Unknown filter method {:}", interlace_method);
    bad_header = true;
  }
  return bad_header?-1:0;
};

std::string to_string(png_pixel_type val){ 
  switch (val){
   using enum png_pixel_type;
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

constexpr std::string to_string(enum libdeflate_result val){
	/* The data would have decompressed to more than 'out_nbytes_avail'
	 * bytes.  */
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
}
    

int png::getData(){
  std::println("Extracting image data...");fflush(0);
  size_t memlen = 0;
  for(png_chunk chunk : index){
    if(chunk.type == chunk_type::IDAT){
      memlen += chunk.length;
    }
  }
  std::println("Compressed size: {:d} bytes",memlen);fflush(0);
  char *compressed_data = reinterpret_cast<char*>(mmap(0,memlen,PROT_READ|PROT_WRITE,MAP_ANONYMOUS|MAP_SHARED,-1,0));
  if(compressed_data == MAP_FAILED){
    std::println("ERR: Faild to mmap memory for the compressed image data");
    return -1;
  };
  char *cur_addr = compressed_data;
  for(png_chunk chunk : index){
    if(chunk.type == chunk_type::IDAT){
      memcpy(cur_addr, chunk.data, chunk.length);
      cur_addr += chunk.length;
    }
  }
  std::println("Copied image data to input buffer for decompression");
  libdeflate_decompressor *decomp_ptr = libdeflate_alloc_decompressor();
  uint64_t img_mem_len;
  if(((width * bit_depth) % 8) == 0){
    img_mem_len = height*(((width * bit_depth)/8) + 1);//byte aligned so no partial byte but add one for filter byte 
  } else {
    img_mem_len = height * (((width*bit_depth)/8) + 2); //compensate for partial byte being truncated due to intger division and for filter byte
  }
  std::println("Calculated Image size:\t{:d} bytes",img_mem_len);
  img_addr = reinterpret_cast<char*>(mmap(0, img_mem_len, PROT_READ|PROT_WRITE,MAP_ANONYMOUS|MAP_SHARED,-1,0));
  size_t bytesout = 0;
  enum libdeflate_result result = libdeflate_zlib_decompress(decomp_ptr, compressed_data,memlen,img_addr,img_mem_len,&bytesout);
  std::println("libdeflate result: {:}", to_string(result));
  std::println("Actual Image size:\t{:d} bytes",bytesout);
  munmap( compressed_data, memlen);
  return 0;
}




