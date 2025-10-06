#include "png.h"
#include "view.h"

extern framebuffer fb;
static constexpr void bswap(auto *val){
  *val = std::byteswap(*val);
}

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
  } else{
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
    read(buf.data(),4);
    if(buf == chunk_type["PLTE"]){
      dprf("Chunk type PLTE\n");
      decodePalette(length);
      dprf("PLTE end\n");
    } else if(buf == chunk_type["IDAT"]){
      dprf("Chunk type IDAT\n");
      decodeImageData(length);
    } else if(buf == chunk_type["IEND"]){
      dprf("Chunk type IEND\n");
      ended = true;
    } else {
      dprf("Unkown chunk, with length {:}\n", length);
      char *dummybuf = new char[length];
      read(dummybuf, length);
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
  uint64_t byte_width = 0;//bytes per scanline
  if( stats.color_type == color_type::greyscale ||
      stats.color_type == color_type::greyscale_alpha || 
      stats.color_type == color_type::palette ){
    bpp = stats.bit_depth;
  } else if ( static_cast<uint8_t>(stats.color_type) & 2 ) {//truecolor
    bpp = stats.bit_depth * 3;
  }
  if( static_cast<uint8_t>(stats.color_type) & 4 ){//has alpha
    bpp += stats.bit_depth;
  }
  int bps = bpp * stats.width+1;//filter byte
  if(bps%8 == 0){
    byte_width = bps/8;
  } else {//integer divison truncs, so +1 account for needing an half used byte.
    byte_width = 1+ bps/8;
  }
  curline.resize(byte_width);
  prevline.assign(byte_width,0);
  scanline_mem = byte_width; 
  return bad_header?-1:0;
}

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

std::string zlib_return_string(int val){
  std::string ret;
  if(val< 0){ ret = "ERR: zlib returned error"; }
  if(val==Z_OK){ret		="okay (Z_OK)"; } else
  if(val==Z_STREAM_END){ret	="stream end (Z_STREAM_END)";} else 
  if(val==Z_NEED_DICT){ret 	="needs dictionary (Z_NEED_DICT)";} else
  if(val==Z_ERRNO){ret	 	="(Z_ERRNO), check errno ";} else
  if(val==Z_STREAM_ERROR){ret	="(Z_STREAM), stream error";} else
  if(val==Z_DATA_ERROR){ret	="(Z_DATA), data error";} else
  if(val==Z_MEM_ERROR){ret	="(Z_MEM), MEM error";} else
  if(val==Z_BUF_ERROR){ret	="(Z_BUF), BUF error";} else
  if(val==Z_VERSION_ERROR){ret	="(Z_VERSION), VERSION error";} 
  return ret;
}
int png::decodeImageData(uint32_t length){
  dprf("Decoding image data...\n");
  int bytes_avail = length;
  int inlen = getpagesize();
  int outlen = 9*getpagesize();
  auto bufin = new uint8_t[inlen];
  auto bufout = new uint8_t[outlen];    
  z_stream zstream = {
    .next_in = bufin,
    .avail_in = 0,
    .next_out = bufout,
    .avail_out = static_cast<unsigned int>(outlen),
    .zalloc = Z_NULL,
    .zfree = Z_NULL,
    .opaque = Z_NULL,
  };
  zstream.avail_in = read(bufin,std::min(bytes_avail, inlen));
  bytes_avail -= zstream.avail_in;
  inflateInit2(&zstream,0);
  dprf("Zstream initialized\n");
  int cnt = 1;
  int ret = inflate(&zstream, Z_SYNC_FLUSH);
  while ((ret>=0)&&(ret!=Z_STREAM_END)){
  //                           avail_out
  // 0                          |--^--| outlen
  // [DDDDDDDDDDDDDDDDDDDDDDDDDD       ]
  // [ccccccccccccccccccccccDDDD       ]
  // [DDDD
    int consumed = filterline(bufout, outlen - zstream.avail_out);
    int leftoverlen = outlen - zstream.avail_out - consumed;
    memmove(bufout, zstream.next_out - leftoverlen, leftoverlen);
    zstream.avail_out += consumed;
    zstream.next_out = bufout + leftoverlen;
    if(zstream.avail_in == 0){
      //dprf("{:} of {:} done{:}\n", zstream.total_in,length,bytes_avail);
    zstream.avail_in = read(bufin,std::min(bytes_avail, inlen));
    bytes_avail -= zstream.avail_in;
    zstream.next_in = bufin;
    };
    ret = inflate(&zstream, Z_SYNC_FLUSH);
  }
  filterline(bufout, outlen - zstream.avail_out);
 // dprf("{:} of {:} done\n", zstream.total_in,length);
  dprf("Zstream ended with return code {:}, total bytes read {:}, length {:}\n",
      zlib_return_string(ret),zstream.total_in,length);
  inflateEnd(&zstream);
  dprf("Zstream closed\n");
  delete[] bufin;
  delete[] bufout;
  return 0;
}

int png::filterline(uint8_t *buf, int length){
  int line = 0;
  int prev_offset = (bpp<8)?1:bpp/8;
  while( line + scanline_mem < length ){
    switch (static_cast<filter_type>(curline[0])){
      using enum filter_type;
      case none:
        break;
      case sub:
        for(int i = 2; i < scanline_mem; i++){//first byte is special
	  curline[i] = buf[i+line] + curline[i-prev_offset];
        }
        break;
      case up:
        for(int i = 1; i < scanline_mem; i++){
	  curline[i] = buf[i+line] + prevline[i];
	}
        break;
      case average:
	for(int i = 2; i < curline.size();i++){
	  curline[i] = buf[i+line] + (static_cast<int>(curline[i-prev_offset])+
		       static_cast<int>(prevline[i-prev_offset]))/2;
	}
	break;
      case paeth:
	curline[1] = buf[1+line] + prevline[1];
        for(int i = 2; i < curline.size(); i++){
	  int a = static_cast<int>(curline[i-prev_offset]);
	  int b = static_cast<int>(prevline[i-prev_offset]);
	  int c = static_cast<int>(prevline[i]);
	  int p = a + b - c;
	  curline[i] = buf[i+line] + std::min({std::abs(p-a), std::abs(p-b), std::abs(p-c)});
	}	
	break;
      default:
	break;
    }
    prevline = curline;
    line += scanline_mem;
  } 
  return line;
}



