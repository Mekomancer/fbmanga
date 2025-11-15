module;
import std;
import std.compat;
#include "debug.h"
#include <cstdint>
import png.util;
export module png;
import types;
/*png (network) to host byte order*/
template <typename T> [[nodiscard]] constexpr T ptoh(T val) noexcept {
  using enum std::endian;
  if constexpr (native == big) {
    return val;
  } else if (native == little) {
    return std::byteswap(val);
  } else {
    static_assert((native == little) || (native == big),
                  "Mixed-endian not supported");
  }
}

/*host to png (network) byte order*/
template <typename T> [[nodiscard]] constexpr T htop(T val) noexcept {
  return ptoh(val);
}
static_assert(htop(ptoh(1)) == 1);

template <typename T>
constexpr T bitscale(T val, int cur, int target) {
  return (((2 * val * ((1 << target) - 1)) / ((1 << cur) - 1)) + 1) / 2;
}

export class png {
public:
  struct __attribute__((__packed__)) ihdr_t {
    uint32_t width;
    uint32_t height;
    uint8_t bit_depth;
    uint8_t color_type;
    uint8_t compression_method;
    uint8_t filter_method;
    uint8_t interlace_method;
  } ihdr;
  static_assert(sizeof(ihdr_t) == 13);
  uint64_t image_size = 0;
  ring_buf in;
  std::vector<rgb888> image;
  int init();
  int parseHead();
  int parsePalette(uint32_t length);
  int decode();

private:
  int chrm(int len);
  int gama(int len);
  double gamma = 1.0;
  int iccp(int len);
  int sbit(int len);
  int srgb(int len);
  int cicp(int len);
  int mdcv(int len);
  int clli(int len);
  int text(int len);
  int ztxt(int len);
  int itxt(int len);
  int bkgd(int len);
  rgb888 background = {0, 0, 0};
  int hist(int len);
  int phys(int len);
  int splt(int len);
  int exif(int len);
  int time(int len);
  int trns(int len);
  void notImplYet(int len);
  std::map<std::string, uint32_t> chunk_type{
      {"IHDR", 0x49'48'44'52}, {"PLTE", 0x50'4C'54'45}, {"IDAT", 0x49'44'41'54},
      {"IEND", 0x49'45'4E'44}, {"tRNS", 0x74'52'4E'52}, {"cHRM", 0x63'48'52'4D},
      {"gAMA", 0x67'41'4D'41}, {"iCCP", 0x69'43'43'50}, {"sBIT", 0x73'42'49'54},
      {"sRGB", 0x73'52'47'42}, {"cICP", 0x63'49'43'50}, {"mDCV", 0x6D'44'43'56},
      {"cLLI", 0x63'4C'4C'49}, {"tEXt", 0x74'45'58'74}, {"zTXt", 0x7A'54'58'74},
      {"iTXt", 0x69'54'58'74}, {"bKGD", 0x62'4B'47'44}, {"hIST", 0x68'49'53'54},
      {"pHYs", 0x70'48'59'73}, {"sPLT", 0x73'50'4C'54}, {"eXIf", 0x65'58'49'66},
      {"tIME", 0x74'49'4D'45},
      // this was all hand typed in, no copy paste, so may be innacurate...
  };
  static constexpr uint64_t signature = 0x89'50'4E'47'0D'0A'1A'0A;
  std::vector<rgb888> palette;
  bool valid_ihdr() noexcept;
  uint32_t checksum = 0;
  bool checkCRC(uint32_t length);
  int decodeImageData(uint32_t length);
  int filterline(const uint8_t *buf, int length);
  std::vector<uint8_t> curline;
  std::vector<uint8_t> prevline;
  int bpp = 0;
  bool validDepthColor();
  bool tainted = false;
  int writeLine();
  int scanline_mem = -1;
}

uint32_t crc32(std::span<uint8_t> dat) {
#if __ARM_FEATURE_CRC32 == true
  return ~std::accumulate(
      dat.begin(), dat.end(), ~(0ul),
      [](uint32_t checksum, uint8_t val) { return __crc32b(checksum, val); });
#else // use zlibs crc
  return crc32_z(uint32_t ret, dat.data(), dat.length());
#endif
}

bool png::validDepthColor() {
  if (!std::has_single_bit(ihdr.bit_depth)) {
    return false;
  }
  if (ihdr.bit_depth == 8) {
    return true;
  }
  if (ihdr.bit_depth == 16 && ihdr.color_type == 3) {
    return false;
  }
  if (ihdr.bit_depth < 8 && !(ihdr.color_type == 3 || ihdr.color_type == 0)) {
    return false;
  };
  return true;
};
int png::parseHead() {
  uint64_t file_sig = ptoh(in.pop<uint64_t>());
  if (file_sig != signature) {
    dprf("ERR: Bad file sig\n");
    dprf("    file sig: {:16x}\n", file_sig);
    dprf("    png  sig: {:16x}\n", signature);
    tainted = true;
  }
  uint32_t len = ptoh(in.pop<uint32_t>());
  if (len != 13) {

    dprf("ERR: Bad IHDR (first chunk's length should be 13, is {:})\n", len);
  };
  if (!checkCRC(len)) {
    ;
    tainted = true;
  };
  uint32_t type = ptoh(in.pop<uint32_t>());
  if (type != chunk_type["IHDR"]) {
    dprf("ERR: First chunk is not IHDR, (got {:8x})\n", type);
    tainted = true;
    return -1;
  }
  ihdr = in.pop<png::ihdr_t>();
  ihdr.width = ptoh(ihdr.width);
  ihdr.height = ptoh(ihdr.height);
  dprf("Width: {:d},", ihdr.width);
  dprf("Height: {:d}, ", ihdr.height);
  dprf("Bit depth: {:d}, ", ihdr.bit_depth);
  dprf("Color type: {:d}\n", ihdr.color_type);
  if (!validDepthColor()) {
    dprf("WARN: invalid color-type and bit-depth combonation");
    tainted = true;
  }
  if (ihdr.compression_method == 0) {
    info("Compression method: DEFLATE\n");
  } else {
    dprf("ERR: Unknown compression method {:}\n", ihdr.compression_method);
    tainted = true;
  }
  if (ihdr.filter_method == 0) {
    dprf("Filter method: Adaptive\n");
  } else {
    dprf("ERR: Unknown filter method {:}\n", ihdr.filter_method);
    tainted = true;
  }
  if (ihdr.interlace_method == 0) {
    dprf("Interlace method: Null\n");
  } else if (ihdr.interlace_method == 1) {
    dprf("Interlace method: Adam7\n");
  } else {
    dprf("ERR: Unknown filter method {:}\n", ihdr.interlace_method);
    tainted = true;
  }
  image_size = static_cast<long>(ihdr.width) * static_cast<long>(ihdr.height);
  bpp = ihdr.bit_depth;
  if (ihdr.color_type == 2) {
    bpp *= 3;
  } else if (ihdr.color_type == 4) {
    bpp *= 2;
  } else if (ihdr.color_type == 6) {
    bpp *= 4;
  }
  scanline_mem = ihdr.width * bpp;
  if (scanline_mem % 8 == 0) {
    scanline_mem = 1 + (scanline_mem / 8);
  } else {
    scanline_mem = 2 + (scanline_mem / 8);
  }
  curline.resize(scanline_mem);
  prevline.resize(scanline_mem);
  in.pop<uint32_t>(); // crc
  return 0;
}

bool png::checkCRC(uint32_t len) {
  std::vector<uint8_t> buf(len + 8 /*crc and chunk type are 4 bytes each*/);
  in.peek<uint8_t>(buf);
  uint32_t calc = crc32(std::span(buf).subspan(0, len + 4));
  uint32_t crc = ptoh(*reinterpret_cast<uint32_t *>(&buf[len + 4]));
  if (crc == calc) {
    return true;
  } else {
    dprf("CRC check failed ToT\ncalculted\t{:8x}\nexpected\t{:8x}\n", calc,
         crc);
    return false;
  }
  return false;
}

int png::init() {
  checksum = ~0;
  image_size = 10;
  return 0;
}

int png::decode() {
  while (in.len() > 0) {
    uint32_t length = ptoh(in.pop<uint32_t>());
    uint32_t buf;
    in.peek(std::span(reinterpret_cast<uint8_t *>(&buf), 4));
    buf = ptoh(buf);
    if (buf == chunk_type["PLTE"]) {
      parsePalette(length);
    } else if (buf == chunk_type["IDAT"]) {
      decodeImageData(length);
    } else if (buf == chunk_type["IEND"]) {
      return 0;
    } else if (buf == chunk_type["IHDR"]) {
      dprf("ERR: more than one IHDR");
    } else if (buf == chunk_type["tRNS"]) {
      trns(length);
    } else if (buf == chunk_type["cHRM"]) {
      chrm(length);
    } else if (buf == chunk_type["gAMA"]) {
      gama(length);
    } else if (buf == chunk_type["iCCP"]) {
      iccp(length);
    } else if (buf == chunk_type["sBIT"]) {
      sbit(length);
    } else if (buf == chunk_type["sRGB"]) {
      srgb(length);
    } else if (buf == chunk_type["cICP"]) {
      cicp(length);
    } else if (buf == chunk_type["mDCV"]) {
      mdcv(length);
    } else if (buf == chunk_type["iTXt"]) {
      itxt(length);
    } else if (buf == chunk_type["tEXt"]) {
      text(length);
    } else if (buf == chunk_type["zTXt"]) {
      ztxt(length);
    } else if (buf == chunk_type["bKGD"]) {
      bkgd(length);
    } else if (buf == chunk_type["hIST"]) {
      hist(length);
    } else if (buf == chunk_type["pHYs"]) {
      phys(length);
    } else if (buf == chunk_type["sPLT"]) {
      splt(length);
    } else if (buf == chunk_type["eXIf"]) {
      exif(length);
    } else if (buf == chunk_type["tIME"]) {
      time(length);
    } else {
      char tmp[] = "err:"; // posion so i can tell if not overwritten
      std::memcpy(tmp, &buf, 4);
      dprf("WARN: unkown chunk (type: {:}, length {:})\n", tmp, length);
      notImplYet(length);
    }
    in.pop<uint32_t>(); // crc
  }
  return 0;
}

int png::parsePalette(uint32_t length) {
  in.pop<uint32_t>();
  if (length % 3 != 0) {
    dprf("ERR: Invalid PLTE chunk, PLTE length ({:d} bytes) is not divisible "
         "by 3",
         length);
    return -1;
  }
  palette.resize(length / 3);
  for (uint i = 0; (i * 3) < length; i++) {
    in.read<rgb888>(palette);
  };
  return 0;
};
int png::decodeImageData(uint32_t length) {
  in.pop<uint32_t>();
  size_t bytes_avail = length;
  size_t inlen = 2 * getpagesize();
  size_t outlen = 2 * getpagesize();
  std::vector<uint8_t> bufin(inlen);
  std::vector<uint8_t> bufout(outlen);
  z_stream zstream;
  zstream.next_in = bufin.data();
  zstream.avail_in = 0;
  zstream.next_out = bufout.data();
  zstream.avail_out = static_cast<unsigned int>(outlen);
  zstream.zalloc = Z_NULL, zstream.zfree = Z_NULL, zstream.opaque = Z_NULL,
  zstream.avail_in = bufin.size();
  bytes_avail -= zstream.avail_in;
  inflateInit2(&zstream, 0);
  int ret = inflate(&zstream, Z_SYNC_FLUSH);
  while ((ret >= 0) && (ret != Z_STREAM_END)) {
    //                           avail_out
    // 0                          |--^--| outlen
    // [DDDDDDDDDDDDDDDDDDDDDDDDDD       ]
    // [ccccccccccccccccccccccDDDD       ]
    // [DDDD
    int consumed = filterline(bufout.data(), outlen - zstream.avail_out);
    int leftoverlen = outlen - zstream.avail_out - consumed;
    memmove(bufout.data(), zstream.next_out - leftoverlen, leftoverlen);
    zstream.avail_out += consumed;
    zstream.next_out = bufout.data() + leftoverlen;
    if (zstream.avail_in == 0) {
      //   dprf("{:} of {:} done{:}\n", zstream.total_in,length,bytes_avail);
      zstream.avail_in = in.read<uint8_t>(std::span<uint8_t>(
          bufin.data(), std::min(bufin.size(), bytes_avail)));
      bytes_avail -= zstream.avail_in;
      zstream.next_in = bufin.data();
    };
    ret = inflate(&zstream, Z_SYNC_FLUSH);
  }
  filterline(reinterpret_cast<uint8_t *>(bufout.data()),
             outlen - zstream.avail_out);
  inflateEnd(&zstream);
  return 0;
}
// c b
// a x <-- byte being (un)filtered
[[nodiscard]] uint8_t constexpr paeth(int a, int b, int c) noexcept {
  int p = a + b - c;
  int pa = std::abs(p - a);
  int pb = std::abs(p - b);
  int pc = std::abs(p - c);
  if (pa <= pb && pa <= pc) {
    return a;
  } else if (pb <= pc) {
    return b;
  } else {
    return c;
  };
}

int png::filterline(const uint8_t *buf, int length) {
  int line = 0;
  int prev_offset = (bpp <= 8) ? 1 : bpp / 8;
  while (line + scanline_mem <= length) {
    curline[0] = 0;
    prevline[0] = 0;
    switch (buf[line]) {
    case 0:
      for (int i = 1; i < scanline_mem; i++) {
        curline[i] = buf[i + line];
      };
      break;
    case 1:
      curline[1] = buf[1 + line];
      for (int i = 2; i < scanline_mem; i++) { // first byte is special
        curline[i] = buf[i + line] + curline[i - prev_offset];
      }
      break;
    case 2:
      for (int i = 1; i < scanline_mem; i++) {
        curline[i] = buf[i + line] + prevline[i];
      }
      break;
    case 3:
      curline[1] = buf[1 + line] + (static_cast<int>(prevline[1])) / 2;
      for (int i = 2; i < scanline_mem; i++) {
        curline[i] =
            buf[i + line] + (static_cast<int>(curline[i - prev_offset]) +
                             static_cast<int>(prevline[i])) /
                                2;
      }
      break;
    case 4:
      curline[1] = buf[1 + line] + paeth(0, prevline[1], 0);
      for (int i = 2; i < scanline_mem; i++) {
        curline[i] =
            buf[i + line] + paeth(static_cast<int>(curline[i - prev_offset]),
                                  static_cast<int>(prevline[i]),
                                  static_cast<int>(prevline[i - prev_offset]));
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

int png::writeLine() {
  if (ihdr.color_type == 3) {
    if (ihdr.bit_depth < 8) {
      uint8_t bmask = (1 << (ihdr.bit_depth)) - 1;
      for (uint32_t i = 8; i < ihdr.width * bpp + 8; i += bpp) {
        uint8_t pindex = std::rotl(curline[i / 8], i + ihdr.bit_depth) & bmask;
        image.push_back(palette[pindex]);
      }
    } else if (ihdr.bit_depth == 8) {
      for (uint i = 1; i < ihdr.width + 1; i++) {
        image.push_back(palette[curline[i]]);
      }
    } else {
      dprf("ERR: Invalid bit depth");
    }
  } else if (ihdr.color_type == 2) {
    if (ihdr.bit_depth == 8) {
      for (int i = 1; i < scanline_mem; i += 3) {
        image.emplace_back(*reinterpret_cast<rgb888 *>(&curline[i]));
      }
    } else if (ihdr.bit_depth == 16) {
      for (uint i = 1; i < ihdr.width * 6 + 1; i += 6) {
        image.emplace_back(curline[i], curline[i + 2], curline[i + 4]);
      }
    }
  } else if (ihdr.color_type == 0) {
    if (ihdr.bit_depth < 8) {
      uint8_t bmask = (1 << (ihdr.bit_depth)) - 1;
      for (uint col = 8; col < ihdr.width * ihdr.bit_depth + 8;
           col += ihdr.bit_depth) {
        uint8_t val = bitscale<uint8_t>(
            std::rotl(curline[col / 8], col + ihdr.bit_depth) & bmask,
            ihdr.bit_depth, 8);
        image.emplace_back(val, val, val);
      }
    } else if (ihdr.bit_depth == 8) {
      for (uint i = 1; i < ihdr.width + 1; i++) {
        image.emplace_back(curline[i], curline[i], curline[i]);
      }
    } else if (ihdr.bit_depth == 16) {
      for (uint i = 1; i < ihdr.width * 2 + 1; i += 2) {
        image.emplace_back(curline[i], curline[i], curline[i]);
      }
    }
  } else if (ihdr.color_type == 4) {
    if (ihdr.bit_depth == 8) {
      for (uint i = 1; i < ihdr.width * 2 + 1; i += 2) {
        image.emplace_back(curline[i], curline[i], curline[i]);
      }
    } else if (ihdr.bit_depth == 16) {
      for (uint i = 1; i < ihdr.width * 4 + 1; i += 4) {
        image.emplace_back(curline[i], curline[i], curline[i]);
      }
    }
  } else if (ihdr.color_type == 6) {
    if (ihdr.bit_depth == 8) {
      for (uint i = 1; i < ihdr.width * 4 + 1; i += 4) {
        image.emplace_back(curline[i], curline[i + ihdr.bit_depth / 8],
                           curline[i + 2]);
      }
    } else if (ihdr.bit_depth == 16) {
      for (uint i = 1; i < ihdr.width * 8 + 1; i += 8) {
        image.emplace_back(curline[i], curline[i + 2], curline[i + 4]);
      }
    }
  }
  return 0;
}

void png::notImplYet(int len) {
  std::vector<uint8_t> dummybuf(len + 8);
  in.read<uint8_t>(dummybuf);
  return;
}
int png::trns(int length) {
  notImplYet(length);
  return 0;
}
int png::chrm(int length) {
  notImplYet(length);
  return 0;
}
int png::gama(int length) {
  notImplYet(length);
  return 0;
}
int png::iccp(int length) {
  notImplYet(length);
  return 0;
}
int png::sbit(int length) {
  notImplYet(length);
  return 0;
}
int png::srgb(int length) {
  notImplYet(length);
  return 0;
}
int png::cicp(int length) {
  notImplYet(length);
  return 0;
}
int png::mdcv(int length) {
  notImplYet(length);
  return 0;
}
int png::itxt(int length) {
  notImplYet(length);
  return 0;
}
int png::text(int length) {
  notImplYet(length);
  return 0;
}
int png::ztxt(int length) {
  notImplYet(length);
  return 0;
}
int png::bkgd(int length) {
  notImplYet(length);
  return 0;
}
int png::hist(int length) {
  notImplYet(length);
  return 0;
}
int png::phys(int length) {
  notImplYet(length);
  return 0;
}
int png::splt(int length) {
  notImplYet(length);
  return 0;
}
int png::exif(int length) {
  notImplYet(length);
  return 0;
}
int png::time(int length) {
  notImplYet(length);
  return 0;
}

int scale(double fctr, std::span<rgb888> image, size_t w, size_t h,
          std::span<rgb888> kernel) {
  double scl = 1 / fctr - 0.1;
  for (uint r = 0; r < h; r++) {
    for (uint c = 0; c < w; c++) {
      long int i = static_cast<int>(c * scl) + 480 * static_cast<int>(r * scl);
      image[r * w + c] = kernel[i];
    }
  }
  return 0;
}
