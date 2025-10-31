#include "png.h"
template<std::integral int_t> /*png (network) to host byte order*/
[[nodiscard]] int_t ptoh(int_t val){
  using enum std::endian;
  if(native == big){
    return val;
  } else if(native == little){
    return std::byteswap(val);
  } else {
    static_assert((native == little)||(native == big),
	"Mixed-endian byte order not supported");
  }
}



void png::crc32(char *data, int len){
  for(int i = 0; i < len; i++){
    checksum = __crc32b(checksum, data[i]);
  }
  return;
}

bool png::validDepthColor(){
  if( !std::has_single_bit(ihdr.bit_depth) ){ return false;}
  if(ihdr.bit_depth == 8  ){ return true; }
  if((ihdr.bit_depth == 16) && (ihdr.color_type == 3 )){ return false; }
  if((ihdr.bit_depth < 8) && !(ihdr.color_type == 3||ihdr.color_type == 0)){
    return false;
  };
  return true;
};


template<typename byte>
int png::getData(byte *buf, size_t num_bytes){
  if (num_bytes > avail_in){
    num_bytes = avail_in;
  }
  memcpy(reinterpret_cast<char *>(buf), next_in, num_bytes);
  crc32(reinterpret_cast<char *>(buf), num_bytes);
  next_in += num_bytes;
  avail_in -= num_bytes;
  return num_bytes;  
}


int png::parseHead(){
  std::array<uint8_t,8> file_sig;
  getData(file_sig.data(), 8);
  if(file_sig != signature){
    dprf("ERR: Bad file sig");
    dprf("    file sig: {:} {:} {:} {:} {:} {:} {:} {:}\n",file_sig[0],file_sig[1],file_sig[2],file_sig[3],file_sig[4],file_sig[5],file_sig[6],file_sig[7]);
    dprf("    png  sig: {:} {:} {:} {:} {:} {:} {:} {:}\n",signature[0],signature[1],signature[2],signature[3],signature[4],signature[5],signature[6],signature[7]);
    tainted = true;
  }
  uint32_t len = 0;
  getData(&len, 4);
  len = ptoh(len);
  if(len != 13){
    dprf("ERR: Bad IHDR (first chunk's length should be 13, is {:})\n",len);
  };
  checksum = ~0;
  std::array<char,4> buf;
  getData(buf.data(),4);
  bool bad_header = false;
  if(buf != chunk_type["IHDR"]){
    dprf("ERR: First chunk is not IHDR, (got {:?}{:?}{:?}{:?})\n",buf[0],buf[1],buf[2],buf[3]);
    tainted = true;
    return -1;
  }
  getData(&ihdr.width,4); 
  ihdr.width = ptoh(ihdr.width);
  dprf("Width: {:}, ", ihdr.width);
  getData(&ihdr.height, 4); 
  ihdr.height = ptoh(ihdr.height);
  dprf("Height: {:}, ", ihdr.height);
  getData(&ihdr.bit_depth);
  dprf("Bit depth: {:}, ", ihdr.bit_depth);
  getData(&ihdr.color_type);
  dprf("Color type: {:}\n", ihdr.color_type);
  getData(&ihdr.compression_method);
  if(!validDepthColor()){
    dprf("WARN: invalid color-type and bit-depth combonation");
    tainted = true;
  }
  if(ihdr.compression_method == 0){
    dprf("Compression method: 0: DEFLATE\n");
  } else {
    dprf("ERR: Unknown compression method {:}\n", ihdr.compression_method);
    tainted = true;
  }
  getData(&ihdr.filter_method);
  if(ihdr.filter_method == 0){
    dprf("Filter method: 0: Adaptive filtering with five basic filter types\n");
  } else {
    dprf("ERR: Unknown filter method {:}\n", ihdr.filter_method);
    tainted = true;
  }
  getData(&ihdr.interlace_method);
  if(ihdr.interlace_method == 0){
    dprf("Interlace method: 0: No interlace\n");
  }else if( ihdr.interlace_method == 1){
    dprf("Interlace method: 1: Adam7 interlace\n");
  } else {
    dprf("ERR: Unknown filter method {:}\n", ihdr.interlace_method);
    tainted = true;
  }
  if(!checkCRC()){
    tainted = true;
  }
  image_size= ihdr.width * ihdr.height;
  bpp = ihdr.bit_depth;
  if( ihdr.color_type == 2 ){
    bpp *= 3;
  } else if ( ihdr.color_type == 4 ) {
    bpp *= 2;
  } else if ( ihdr.color_type == 6 ) {
    bpp *= 4;
  } 
  scanline_mem = ihdr.width*bpp;
  if(scanline_mem % 8 == 0){
    scanline_mem = 1 + ( scanline_mem/8);
  } else {
    scanline_mem = 2 + ( scanline_mem/8);
  }
  curline.resize(scanline_mem);
  prevline.resize(scanline_mem);
  return 0;
}


bool png::checkCRC(){
  uint32_t calculated = (~0)^checksum;
  uint32_t crc;
  getData(&crc, 4); 
  crc = ptoh(crc);
  if( calculated==crc ){
    return true;
  } else {
    dprf("CRC check failed ToT\ncalculted\t{:}\nexpected\t{:}\n",
	calculated, crc);
    return false;
  }
  return false;
}

int png::init(){
  checksum = ~0;
  image_size = 10;
  return 0;
}

int png::decode(){
  dprf("Decoding PNG\n");
  while((avail_in > 0)){
    uint32_t length;
    if(getData(&length,4)==-1){
      return -1;
    }
    length = ptoh(length);
    checksum = ~0;
    std::array<char,4> buf;
    getData(buf.data(),4);
	   if(buf == chunk_type["PLTE"]){ parsePalette(length);
    } else if(buf == chunk_type["IDAT"]){ decodeImageData(length);
    } else if(buf == chunk_type["IEND"]){ return 0;
    } else if(buf == chunk_type["IHDR"]){ dprf("ERR: more than one IHDR");
    } else if(buf == chunk_type["tRNS"]){ trns(length);
    } else if(buf == chunk_type["cHRM"]){ chrm(length);
    } else if(buf == chunk_type["gAMA"]){ gama(length);
    } else if(buf == chunk_type["iCCP"]){ iccp(length);
    } else if(buf == chunk_type["sBIT"]){ sbit(length);
    } else if(buf == chunk_type["sRGB"]){ srgb(length);
    } else if(buf == chunk_type["cICP"]){ cicp(length);
    } else if(buf == chunk_type["mDCV"]){ mdcv(length);
    } else if(buf == chunk_type["iTXt"]){ itxt(length);
    } else if(buf == chunk_type["tEXt"]){ text(length);
    } else if(buf == chunk_type["zTXt"]){ ztxt(length);
    } else if(buf == chunk_type["bKGD"]){ bkgd(length);
    } else if(buf == chunk_type["hIST"]){ hist(length);
    } else if(buf == chunk_type["pHYs"]){ phys(length);
    } else if(buf == chunk_type["sPLT"]){ splt(length);
    } else if(buf == chunk_type["eXIf"]){ exif(length);
    } else if(buf == chunk_type["tIME"]){ time(length);
    } else {
      dprf("WARN: unkown chunk (type: {:}{:}{:}{:}, length {:})\n",buf[0],buf[1],buf[2],buf[3], length);
      char *dummybuf = new char[length];
      getData(dummybuf, length);
    }
    checkCRC();
  };
  return 0;
}

int png::parsePalette(uint32_t length){
  if( length % 3 != 0 ){
    dprf("ERR: Invalid PLTE chunk, PLTE length ({:d} bytes) is not divisible by 3", length);
    return -1;
  }
  palette.resize(length/3);
  for(int i=0; (i*3) < length; i++){
    getData(&palette[i].r,1);
    getData(&palette[i].g,1);
    getData(&palette[i].b,1);
//  dprf("  {:3>d}: {:02X} {:02X} {:02X}\n",i,palette[i].r,palette[i].g,palette[i].b);
  };
  return 0;
};

static constexpr std::string colorTypeString(uint8_t val){ 
  switch (val){
    case 0:
      return "0: Greyscale";
    case 1:
      return "2: Truecolor";//there is no color type 1
    case 3:
      return "3: Indexed Color (palette)";
    case 4:
      return "4: Greyscale with alpha";
    case 6:
      return "6: Truecolor with alpha";//same here, no color type 5
    default:
      return std::format("\nERR: Invalid color type {:d}", val);
  }
}


static constexpr std::string zlib_return_string(int val){
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
  int bytes_avail = length;
  int inlen = 2*getpagesize();
  int outlen = 2*getpagesize();
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
  zstream.avail_in = getData(bufin,std::min(bytes_avail, inlen));

  bytes_avail -= zstream.avail_in;
  inflateInit2(&zstream,0);
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
 //   dprf("{:} of {:} done{:}\n", zstream.total_in,length,bytes_avail);
    zstream.avail_in = getData(bufin,std::min(bytes_avail, inlen));
    bytes_avail -= zstream.avail_in;
    zstream.next_in = bufin;
    };
    ret = inflate(&zstream, Z_SYNC_FLUSH);
  }
  filterline(bufout, outlen - zstream.avail_out);
 // dprf("{:} of {:} done\n", zstream.total_in,length);
//  dprf("Zstream ended with return code {:}, total bytes read {:}, length {:}\n",
 //     zlib_return_string(ret),zstream.total_in,length);
  inflateEnd(&zstream);
  delete[] bufin;
  delete[] bufout;
  return 0;
}
// c b 
// a x <-- byte being (un)filtered
[[nodiscard]] uint8_t constexpr paeth(int a, int b, int c) noexcept{
  int p = a + b - c;
  int pa = std::abs(p-a);
  int pb = std::abs(p-b);
  int pc = std::abs(p-c);
  if (pa <= pb && pa <= pc){
    return a;
  } else if (pb <= pc) {
    return b;
  } else {
    return c;
  };
}

int png::filterline(uint8_t *buf, int length){
  int line = 0;
  int prev_offset = (bpp<=8)?1:bpp/8;
  while( line + scanline_mem <= length ){
    curline[0] = 0;
    prevline[0] = 0;
    dprf("{:}",buf[line]);
    switch (buf[line]){
      case 0:
	for(int i = 1; i < scanline_mem; i++){
	  curline[i] = buf[i+line];
	};
        break;
      case 1:
	curline[1] = buf[1+line];
        for(int i = 2; i < scanline_mem; i++){//first byte is special
	  curline[i] = buf[i+line] + curline[i-prev_offset];
        }
        break;
      case 2:
        for(int i = 1; i < scanline_mem; i++){
	  curline[i] = buf[i+line] + prevline[i];
	}
        break;
      case 3:
	curline[1] = buf[1+line] + (static_cast<int>(prevline[1]))/2;
	for(int i = 2; i < scanline_mem;i++){
	  curline[i] = buf[i+line] + (static_cast<int>(curline[i-prev_offset])+
		       static_cast<int>(prevline[i]))/2;
	}
	break;
      case 4:
	curline[1] =  buf[1+line] + paeth(0,prevline[1],0);
        for(int i = 2; i < scanline_mem; i++){
	  curline[i]=buf[i+line] + 
	    paeth(static_cast<int>(curline[i-prev_offset]),
		  static_cast<int>(prevline[i]),
		  static_cast<int>(prevline[i-prev_offset]));
	}	
	break;
      default:
	dprf("Warn: unknown filter type encountered ({:})\n", buf[line]);
	tainted = true;
	break;
    }
    writeLine();
    prevline = curline;
    line += scanline_mem;
  } 
  return line;
}

template<typename byte>
int png::putData(byte *buf, size_t num_bytes){
  if (num_bytes > avail_out){
    return -1;
  }
  char *buffer = reinterpret_cast<char *>(buf);
  memcpy(next_out, buffer, num_bytes);
  next_out += num_bytes;
  avail_out -= num_bytes;
  return num_bytes;  
}
int png::writeLine(){
  if(ihdr.color_type == 3){
    if(ihdr.bit_depth < 8){
      uint8_t bmask = (1<<(ihdr.bit_depth))-1;
  //    dprf("bitdepth = {:}, bitmask = {:b}\n",ihdr.bit_depth, bits);
      for(uint32_t i = 8; i < ihdr.width * bpp + 8; i+=bpp){
	uint8_t pindex = std::rotl(curline[i/8],i+ihdr.bit_depth) & bmask;
//	dprf("{:}({:} ({:} {:b})), ",i,curline[i/8],pindex,bitmask);
	putData(&(palette[pindex].r),1);
	putData(&(palette[pindex].g),1);
	putData(&(palette[pindex].b),1);
      }
    } else if(ihdr.bit_depth == 8) {
      for(int i = 1; i < ihdr.width+1; i++){
	putData(&(palette[curline[i]].r),1);
	putData(&(palette[curline[i]].g),1);
	putData(&(palette[curline[i]].b),1);
      }
    } else {
      dprf("ERR: Invalid bit depth");
    }
  } else if(ihdr.color_type == 2){
    if(ihdr.bit_depth == 8){
      for(int i = 1; i < scanline_mem; i+=3){
	putData(&(curline[i]),3);
      }
    } else if( ihdr.bit_depth == 16){
      for(int i = 1; i < ihdr.width*6 + 1 ; i+=6){
	putData(&(curline[i]),1);
	putData(&(curline[i+2]),1);
	putData(&(curline[i+4]),1);
      }
    }
  } else if(ihdr.color_type == 0){
    if(ihdr.bit_depth < 8){
      int offset = 0;
      uint8_t bmask = (1<<(ihdr.bit_depth))-1;
      for(int col = 8; col < ihdr.width*ihdr.bit_depth +8; col += ihdr.bit_depth){
	uint8_t val = bitscale<uint8_t>(
	  std::rotl(curline[col/8],col+ihdr.bit_depth) & bmask,
	  ihdr.bit_depth,
	  8
        );
	putData(&val,1);
	putData(&val,1);
	putData(&val,1);
      }
    } else if(ihdr.bit_depth == 8){
      for(int i = 1; i < ihdr.width + 1; i++){
	putData(&(curline[i]),1);
	putData(&(curline[i]),1);
	putData(&(curline[i]),1);
      }
    } else if(ihdr.bit_depth == 16){
      for(int i = 1; i < ihdr.width*2 + 1 ; i+=2){
	putData(&(curline[i]),1);
	putData(&(curline[i]),1);
	putData(&(curline[i]),1);
      }
    }    
  } else if(ihdr.color_type == 4) {
   if(ihdr.bit_depth == 8){
      for(int i = 1; i < ihdr.width*2 +1; i+=2){
	putData(&(curline[i]),1);
	putData(&(curline[i]),1);
	putData(&(curline[i]),1);
      }
    } else if(ihdr.bit_depth == 16){
      for(int i = 1; i < ihdr.width*4 + 1 ; i+=4){
	putData(&(curline[i]),1);
	putData(&(curline[i]),1);
	putData(&(curline[i]),1);
      }
    }    
  } else if(ihdr.color_type == 6) {
   if(ihdr.bit_depth == 8){
      for(int i = 1; i < ihdr.width*4 +1; i+=4){
	putData(&(curline[i]),1);
	putData(&(curline[i+1]),1);
	putData(&(curline[i+2]),1);
      }
    } else if(ihdr.bit_depth == 16){
      for(int i = 1; i < ihdr.width*8 + 1 ; i+=8){
	putData(&(curline[i]),1);
	putData(&(curline[i+2]),1);
	putData(&(curline[i+4]),1);
      }
    }    
  }
  return 0;
}


int png::trns(int length){
  char *dummybuf = new char[length];
  getData(dummybuf, length);
  return 0;
}
int png::chrm(int length){
  char *dummybuf = new char[length];
  getData(dummybuf, length);
  return 0;
}
int png::gama(int length){
  char *dummybuf = new char[length];
  getData(dummybuf, length);
  return 0;
}
int png::iccp(int length){
  char *dummybuf = new char[length];
  getData(dummybuf, length);
  return 0;
}
int png::sbit(int length){
  char *dummybuf = new char[length];
  getData(dummybuf, length);
  return 0;
}
int png::srgb(int length){
  char *dummybuf = new char[length];
  getData(dummybuf, length);
  return 0;
}
int png::cicp(int length){
  char *dummybuf = new char[length];
  getData(dummybuf, length);
  return 0;
}
int png::mdcv(int length){
  char *dummybuf = new char[length];
  getData(dummybuf, length);
  return 0;
}
int png::itxt(int length){
  char *dummybuf = new char[length];
  getData(dummybuf, length);
  return 0;
}
int png::text(int length){
  char *dummybuf = new char[length];
  getData(dummybuf, length);
  return 0;
}
int png::ztxt(int length){
  char *dummybuf = new char[length];
  getData(dummybuf, length);
  return 0;
}
int png::bkgd(int length){
  char *dummybuf = new char[length];
  getData(dummybuf, length);
  return 0;
}
int png::hist(int length){
  char *dummybuf = new char[length];
  getData(dummybuf, length);
  return 0;
}
int png::phys(int length){
      char *dummybuf = new char[length];
      getData(dummybuf, length);
  return 0;
}
int png::splt(int length){
  char *dummybuf = new char[length];
  getData(dummybuf, length);
  return 0;
}
int png::exif(int length){
  char *dummybuf = new char[length];
  getData(dummybuf, length);
  return 0;
}
int png::time(int length){
  char *dummybuf = new char[length];
  getData(dummybuf, length);
  return 0;
}
