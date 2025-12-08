module;
export module inflate;
import std;
import types;
import util;
namespace zlib {
export struct __attribute__((packed)) Cmf {
  uint8_t method : 4; // 0-3
  uint8_t info : 4;   // 4-7
};
static_assert(sizeof(Cmf) == 1);

export constexpr Cmf PNG_CMF{8, 7};

export struct __attribute__((packed)) Flg {
  uint8_t check : 5;      // 0-4
  uint8_t dictionary : 1; // 5
  uint8_t level : 2;      // 6-7
};
export struct __attribute__((packed)) Header {
  Cmf cmf;
  Flg flg;
};
static_assert(sizeof(Flg) == 1);
export bool is_valid_png_zlib_header(uint16_t data) {
  if (data % 31 != 0) {
    return false;
  }
  Header header = std::bit_cast<Header>(data);
  if (header.cmf.method != 8) {
    return false;
  }
  if (header.cmf.info > 7) {
    return false;
  }
  if (header.flg.dictionary == 1) {
    return false;
  }
  return true;
};

}; // namespace zlib
