#include "png.h"
#include "util.h"

template <typename t>
  requires(sizeof(t) == 1)
uint32_t crc32(std::span<t> dat) {
  uint32_t ret = ~0;
#if __ARM_FEATURE_CRC32 == 1
  // use intrisics
  for (uint i = 0; i < dat.size(); i++) {
    ret = __crc32b(ret, static_cast<char>(dat[i]));
  }
#else
  // use zlibs crc
  crc32_z(uint32_t ret, dat.data(), dat.length());
#endif
  return ~ret;
}

bool png::validDepthColor() {
  if (!std::has_single_bit(ihdr.bit_depth)) {
    return false;
  }
  if (ihdr.bit_depth == 8) {
    return true;
  }
  if ((ihdr.bit_depth == 16) && (ihdr.color_type == 3)) {
    return false;
  }
  if ((ihdr.bit_depth < 8) && !(ihdr.color_type == 3 || ihdr.color_type == 0)) {
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
    return -1;
  };
  uint32_t type = ptoh(in.pop<uint32_t>());
  if (type != chunk_type["IHDR"]) {
    dprf("ERR: First chunk is not IHDR, (got {:8x})\n", type);
    tainted = true;
    return -1;
  }
  ihdr = in.pop<png::ihdr_t>();
  ihdr.width = ptoh(ihdr.width);
  dprf("Width: {:d}, ", ihdr.width);
  ihdr.height = ptoh(ihdr.height);
  dprf("Height: {:d}, ", ihdr.height);
  dprf("Bit depth: {:d}, ", ihdr.bit_depth);
  dprf("Color type: {:d}\n", ihdr.color_type);
  if (!validDepthColor()) {
    dprf("WARN: invalid color-type and bit-depth combonation");
    tainted = true;
  }
  if (ihdr.compression_method == 0) {
    dprf("Compression method: 0: DEFLATE\n");
  } else {
    dprf("ERR: Unknown compression method {:}\n", ihdr.compression_method);
    tainted = true;
  }
  if (ihdr.filter_method == 0) {
    dprf("Filter method: 0: Adaptive filtering with five basic filter types\n");
  } else {
    dprf("ERR: Unknown filter method {:}\n", ihdr.filter_method);
    tainted = true;
  }
  if (ihdr.interlace_method == 0) {
    dprf("Interlace method: 0: No interlace\n");
  } else if (ihdr.interlace_method == 1) {
    dprf("Interlace method: 1: Adam7 interlace\n");
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
  return 0;
}

bool png::checkCRC(uint32_t len) {
  std::vector<uint8_t> buf(len + 8 /*crc and chunk type are 4 bytes each*/);
  in.peek<uint8_t>(buf);
  uint32_t calc = crc32<uint8_t>(std::span(buf).subspan(0, len + 4));
  uint32_t crc = *reinterpret_cast<uint32_t *>(buf[len + 3]);
  if (crc == calc) {
    return true;
  } else {
    dprf("CRC check failed ToT\ncalculted\t{:}\nexpected\t{:}\n", calc, crc);
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
  uint32_t length = ptoh(in.pop<uint32_t>());
  uint32_t buf = ptoh(in.pop<uint32_t>());
  while (in.len() > 0) {
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
      dprf("WARN: unkown chunk (type: {:8x}, length {:})\n", buf, length);
      notImplYet(length);
    }
  }
  return 0;
}

int png::parsePalette(uint32_t length) {
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
    dprf("{:}", buf[line]);
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
        image.emplace_back(curline[i], curline[i + 1], curline[i + 2]);
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
  std::vector<uint8_t> dummybuf;
  dummybuf.resize(len + 4);
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

extern frame_buffer fb;
int display(std::span<rgb888> image, int scroll) {
  for (uint col = 0; col < fb.vinfo.yres; col++) {
    for (uint row = 0; row < fb.vinfo.xres; row++) {
      if (0 < row + scroll && (row + scroll * 420 + col) < image.size()) {
        fb.setPixel(fb.vinfo.yres - 1 - col, row,
                    image[(row + scroll) * 420 + col]);
      }
    }
  }
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
