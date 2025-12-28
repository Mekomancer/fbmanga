module;
#include <arm_acle.h>
#include <zlib.h>
export module png;
import debug;
import util;
import std;
import types;
import gui;

export enum class PngCode {
  ok,
  bad_file_sig,
  bad_ihdr,
  invalid_checksum,
};
export class Reader {};
export class Png {
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
  image_t image;
  int init();
  PngCode parseHead();
  int parsePalette(uint32_t length);
  int decode();

private:
  // chunk handlers
  void notImplYet(int len);
  int chrm(int len);
  int gama(int len);
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
  int hist(int len);
  int phys(int len);
  int splt(int len);
  int exif(int len);
  int time(int len);
  int trns(int len);

  rgb888 background = {0, 0, 0};
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

  double gamma = 1.0;

  std::vector<rgb888> palette;
  bool checkCRC(uint32_t length);
  int decodeImageData(uint32_t length);
  int filterline(const uint8_t *buf, int length, int row);
  std::vector<uint8_t> curline;
  std::vector<uint8_t> prevline;
  int bpp = 0;
  bool validDepthColor();
  bool tainted = false;
  int writeLine(int row);
  int scanline_mem = -1;
};

bool Png::validDepthColor() {
  //   | 0 | 1 | 2 | 3 | 4 | 5 | 6 |
  //  1| x |   |   | x |   |   |   |1
  //  2| x |   |   | x |   |   |   |2
  //  4| x |   |   | x |   |   |   |4
  //  8| x |   | x | x | x |   | x |8
  // 16| x |   | x |   | x |   | x |16
  switch (ihdr.color_type) {
  case 0:
  case 2:
  case 3:
  case 4:
  case 6:
    break;
  default:
    return false;
  }
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

PngCode Png::parseHead() {
  log("info: Parsing PNG header\n", here());
  uint64_t file_sig = ptoh(in.pop<uint64_t>());
  log("dump: file sig: {:16x}\n", here(), file_sig);
  log("dump: png  sig: {:16x}\n", here(), signature);
  if (file_sig != signature) {
    log("error: Bad file sig\n", here());
    return PngCode::bad_file_sig;
  }
  uint32_t len = ptoh(in.pop<uint32_t>());
  log("dump: IHDR length: {:}", here(), len);
  if (len != 13) {
    log("error: Bad IHDR (length should be 13, is {:})\n", here(), len);
    return PngCode::bad_ihdr;
  };
  if (!checkCRC(len)) {
    return PngCode::invalid_checksum;
  };
  uint32_t type = ptoh(in.pop<uint32_t>());
  if (type != chunk_type["IHDR"]) {
    log("error: First chunk's type is not IHDR, (got {:8x})\n", here(), type);
    return PngCode::bad_ihdr;
  }
  ihdr = in.pop<Png::ihdr_t>();
  ihdr.width = ptoh(ihdr.width);
  ihdr.height = ptoh(ihdr.height);
  log("info: Width: {:d}, Height: {:d}, Bit depth: {:d}, Color type: {:d}",
      here(), ihdr.width, ihdr.height, ihdr.bit_depth, ihdr.color_type);
  if (!validDepthColor()) {
    log("warn: invalid color-type and bit-depth combonation", here());
    return PngCode::bad_ihdr;
  }
  if (ihdr.compression_method == 0) {
    log("info: Compression method: DEFLATE\n", here());
  } else {
    log("warn: Unknown compression method {:}\n", here(),
        ihdr.compression_method);
  }
  if (ihdr.filter_method == 0) {
    log("info: Filter method: Adaptive\n", here());
  } else {
    log("warn: Unknown filter method {:}\n", here(), ihdr.filter_method);
  }
  if (ihdr.interlace_method == 0) {
    log("info: Interlace method: Null\n", here());
  } else if (ihdr.interlace_method == 1) {
    log("info: Interlace method: Adam7\n", here());
  } else {
    log("warn: Unknown filter method {:}\n", here(), ihdr.interlace_method);
  }
  image_size = static_cast<long>(ihdr.width) * static_cast<long>(ihdr.height);
  image.resize(ihdr.width, ihdr.height);
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
  return PngCode::ok;
}

bool Png::checkCRC(uint32_t len) {
  log("info: Checking CRC, length={:}\n", here(), len);
  std::vector<uint8_t> buf(len + 8 /*crc and chunk type are 4 bytes each*/);
  in.peek<uint8_t>(buf);
  uint32_t calc = crc32(std::span(buf).subspan(0, len + 4));
  uint32_t crc = ptoh(*reinterpret_cast<uint32_t *>(&buf[len + 4]));
  if (crc == calc) {
    log("info: CRC check passed ^.^ (calc: {:8x}, png: {:8x})\n", here(), calc,
        crc);
    return true;
  } else {
    log("warn: CRC check failed ToT (calc: {:8x}, png: {:8x})\n", here(), calc,
        crc);
    return false;
  }
  return false;
}

int Png::init() {
  image_size = 10;
  log("info: PNG init'ed\n", here());
  return 0;
}

int Png::decode() {
  log("info: Decoding PNG\n", here());
  while (in.len() > 0) {
    uint32_t length = ptoh(in.pop<uint32_t>());
    log("dump: Chunk length: {:}\n", here(), length);
    uint32_t buf;
    in.peek(std::span(reinterpret_cast<uint8_t *>(&buf), 4));
    buf = ptoh(buf);
    log("dump: Chunk type: {:8x}\n", here(), buf);
    if (buf == chunk_type["PLTE"]) {
      parsePalette(length);
    } else if (buf == chunk_type["IDAT"]) {
      decodeImageData(length);
    } else if (buf == chunk_type["IEND"]) {
      log("info: Reached IEND, decoding finished\n", here());
      return 0;
    } else if (buf == chunk_type["IHDR"]) {
      log("error: more than one IHDR\n", here());
      return -1;
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
      char tmp[] = "4242";
      std::memcpy(tmp, &buf, 4);
      log("warn: unkown chunk (type: {:}, length {:})\n", here(), tmp, length);
      notImplYet(length);
    }
    in.pop<uint32_t>(); // crc
    log("dump: Bytes left: {:}\n", here(), in.len());
  }
  return 0;
}

int Png::parsePalette(uint32_t length) {
  in.pop<uint32_t>();
  if (length % 3 != 0) {
    log("error: Invalid PLTE chunk, PLTE length ({:d}) is not divisible by 3\n",
        here(), length);
    return -1;
  }
  palette.resize(length / 3);
  for (uint32_t i = 0; (i * 3) < length; i++) {
    in.read<rgb888>(palette);
  };
  return 0;
};
int Png::decodeImageData(uint32_t length) {
  log("info: Decoding image data\n", here());
  in.pop<uint32_t>();
  size_t bytes_avail = length;
  size_t inlen = 2 * getpagesize();
  size_t outlen = 2 * getpagesize();
  std::vector<uint8_t> bufin(inlen);
  std::vector<uint8_t> bufout(outlen);
  int row = 0;
  z_stream zstream;
  zstream.next_in = bufin.data();
  zstream.avail_in = 0;
  zstream.next_out = bufout.data();
  zstream.avail_out = static_cast<unsigned int>(outlen);
  zstream.zalloc = Z_NULL, zstream.zfree = Z_NULL, zstream.opaque = Z_NULL,
  zstream.avail_in = in.read<uint8_t>(bufin);
  bytes_avail -= zstream.avail_in;
  inflateInit2(&zstream, 0);
  int ret = inflate(&zstream, Z_SYNC_FLUSH);
  while ((ret >= 0) && (ret != Z_STREAM_END)) {
    //                           avail_out
    // 0                          |--^--| outlen
    // [DDDDDDDDDDDDDDDDDDDDDDDDDD       ]
    // [ccccccccccccccccccccccDDDD       ]
    // [DDDD
    int consumed = filterline(bufout.data(), outlen - zstream.avail_out, row);
    ++row;
    int leftoverlen = outlen - zstream.avail_out - consumed;
    std::memmove(bufout.data(), zstream.next_out - leftoverlen, leftoverlen);
    zstream.avail_out += consumed;
    zstream.next_out = bufout.data() + leftoverlen;
    if (zstream.avail_in == 0) {
      //      log("dump: {:} of {:} done{:}\n", here(), zstream.total_in,
      //      length, bytes_avail);
      zstream.avail_in = in.read<uint8_t>(std::span<uint8_t>(
          bufin.data(), std::min(bufin.size(), bytes_avail)));
      bytes_avail -= zstream.avail_in;
      zstream.next_in = bufin.data();
    };
    ret = inflate(&zstream, Z_SYNC_FLUSH);
  }
  filterline(reinterpret_cast<uint8_t *>(bufout.data()),
             outlen - zstream.avail_out, row);
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

int Png::filterline(const uint8_t *buf, int length, int row) {
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
      log("warn: unknown filter type encountered ({:})\n", here(), buf[line]);
      tainted = true;
      break;
    }
    writeLine(row);
    prevline = curline;
    line += scanline_mem;
  }
  return line;
}

int Png::writeLine(int row) {
  if (ihdr.color_type == 3) {
    if (ihdr.bit_depth < 8) {
      uint8_t bmask = (1 << (ihdr.bit_depth)) - 1;
      int col = 0;
      for (uint32_t i = 8; i < ihdr.width * bpp + 8; i += bpp) {
        uint8_t pindex = std::rotl(curline[i / 8], i + ihdr.bit_depth) & bmask;
        image[row, col++] = palette[pindex];
      }
    } else if (ihdr.bit_depth == 8) {
      int col = 0;
      for (uint32_t i = 1; i < ihdr.width + 1; i++) {
        image[row, col++] = palette[curline[i]];
      }
    } else {
      log("error: Invalid bit depth\n", here());
    }
  } else if (ihdr.color_type == 2) {
    if (ihdr.bit_depth == 8) {
      int col = 0;
      for (int i = 1; i < scanline_mem; i += 3) {
        image[row, col++] = *reinterpret_cast<rgb888 *>(&curline[i]);
      }
    } else if (ihdr.bit_depth == 16) {
      int col = 0;
      for (uint32_t i = 1; i < ihdr.width * 6 + 1; i += 6) {
        image[row, col++] = rgb888(curline[i], curline[i + 2], curline[i + 4]);
      }
    }
  } else if (ihdr.color_type == 0) {
    if (ihdr.bit_depth < 8) {
      uint8_t bmask = (1 << (ihdr.bit_depth)) - 1;
      int c = 0;
      for (uint32_t col = 8; col < ihdr.width * ihdr.bit_depth + 8;
           col += ihdr.bit_depth) {
        uint8_t val = bitscale<uint8_t>(
            std::rotl(curline[col / 8], col + ihdr.bit_depth) & bmask,
            ihdr.bit_depth, 8);
        image[row, c++] = rgb888(val, val, val);
      }
    } else if (ihdr.bit_depth == 8) {
      int col = 0;
      for (uint32_t i = 1; i < ihdr.width + 1; ++i) {
        image[row, col++] = rgb888(curline[i], curline[i], curline[i]);
      }
    } else if (ihdr.bit_depth == 16) {
      int col = 0;
      for (uint32_t i = 1; i < ihdr.width * 2 + 1; i += 2) {
        image[row, col++] = rgb888(curline[i], curline[i], curline[i]);
      }
    }
  } else if (ihdr.color_type == 4) {
    if (ihdr.bit_depth == 8) {
      int col = 0;
      for (uint32_t i = 1; i < ihdr.width * 2 + 1; i += 2) {
        image[row, col++] = rgb888(curline[i], curline[i], curline[i]);
      }
    } else if (ihdr.bit_depth == 16) {
      int col = 0;
      for (uint32_t i = 1; i < ihdr.width * 4 + 1; i += 4) {
        image[row, col++] = rgb888(curline[i], curline[i], curline[i]);
      }
    }
  } else if (ihdr.color_type == 6) {
    if (ihdr.bit_depth == 8) {
      int col = 0;
      for (uint32_t i = 1; i < ihdr.width * 4 + 1; i += 4) {
        image[row, col++] =
            rgb888(curline[i], curline[i + ihdr.bit_depth / 8], curline[i + 2]);
      }
    } else if (ihdr.bit_depth == 16) {
      int col = 0;
      for (uint32_t i = 1; i < ihdr.width * 8 + 1; i += 8) {
        image[row, col++] = rgb888(curline[i], curline[i + 2], curline[i + 4]);
      }
    }
  }
  return 0;
}

void Png::notImplYet(int len) {
  log("warn: Chunk handler not implemeted yet, skipping chunk\n", here());
  std::vector<uint8_t> dummybuf(len + 4);
  in.read<uint8_t>(dummybuf);
  log("info: Skipped {:} bytes\n", here(), dummybuf.size());
  return;
}
int Png::trns(int length) {
  log("info: Decoding transparency info (type: tRNS length: {:})\n", here(),
      length);
  notImplYet(length);
  return 0;
}
int Png::chrm(int length) {
  log("info: Decoding chroma info, (type: cHRM, length: {:})\n", here(),
      length);
  notImplYet(length);
  return 0;
}
int Png::gama(int length) {
  log("info: Decoding gamma info, (type: gAMA, length: {:}\n", here(), length);
  notImplYet(length);
  return 0;
}
int Png::iccp(int length) {
  log("info: Decoding ICC profile, (type: iCCP, length: {:}\n", here(), length);
  notImplYet(length);
  return 0;
}
int Png::sbit(int length) {
  log("info: Decoding sample depth info, (type: sBIT, length: {:}\n", here(),
      length);
  notImplYet(length);
  return 0;
}
int Png::srgb(int length) {
  log("info: Decoding sRGB info, (type: sRGB, length: {:}\n", here(), length);
  notImplYet(length);
  return 0;
}
int Png::cicp(int length) {
  log("info: Decoding color info, (type: cICP, length: {:}\n", here(), length);
  notImplYet(length);
  return 0;
}
int Png::mdcv(int length) {
  log("info: Decoding color info, (type: mDCV, length: {:}\n", here(), length);
  notImplYet(length);
  return 0;
}
int Png::itxt(int length) {
  log("info: Decoding text, (type: iTXT, length: {:}\n", here(), length);
  notImplYet(length);
  return 0;
}
int Png::text(int length) {
  log("info: Decoding gamma info, (type: tEXT, length: {:}\n", here(), length);
  notImplYet(length);
  return 0;
}
int Png::ztxt(int length) {
  log("info: Decoding gamma info, (type: zTXT, length: {:}\n", here(), length);
  notImplYet(length);
  return 0;
}
int Png::bkgd(int length) {
  log("info: Decoding gamma info, (type: bKGD, length: {:}\n", here(), length);
  notImplYet(length);
  return 0;
}
int Png::hist(int length) {
  log("info: Decoding gamma info, (type: hIST, length: {:}\n", here(), length);
  notImplYet(length);
  return 0;
}
int Png::phys(int length) {
  log("info: Decoding gamma info, (type: pHYS, length: {:}\n", here(), length);
  notImplYet(length);
  return 0;
}
int Png::splt(int length) {
  log("info: Decoding gamma info, (type: sPLT, length: {:}\n", here(), length);
  notImplYet(length);
  return 0;
}
int Png::exif(int length) {
  log("info: Decoding gamma info, (type: eXIF, length: {:}\n", here(), length);
  notImplYet(length);
  return 0;
}
int Png::time(int length) {
  log("info: Decoding gamma info, (type: tIME, length: {:}\n", here(), length);
  notImplYet(length);
  return 0;
}
