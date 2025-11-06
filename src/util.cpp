#include "util.h"

bool constexpr ring_buf::is_wrapped() {
  if (end < begin) {
    return true;
  } else if (begin == end) {
    return false;
  } else if (end > begin) {
    return false;
  } else {
    return true; // if wrapped, extra stuff is done, hopefully catching errors
  }
}
size_t ring_buf::len() {
  if (is_wrapped()) {
    return end + (data.size() - (begin + 1));
  } else {
    return end - begin;
  }
}

constexpr std::string colorTypeString(uint8_t val) {
  switch (val) {
  case 0:
    return "0: Greyscale";
  case 1:
    return "2: Truecolor"; // there is no color type 1
  case 3:
    return "3: Indexed Color (palette)";
  case 4:
    return "4: Greyscale with alpha";
  case 6:
    return "6: Truecolor with alpha"; // same here, no color type 5
  default:
    return std::format("\nERR: Invalid color type {:d}", val);
  }
}

constexpr std::string zlib_return_string(int val) {
  std::string ret;
  if (val < 0) {
    ret = "ERR: zlib returned error";
  }
  if (val == Z_OK) {
    ret = "okay (Z_OK)";
  } else if (val == Z_STREAM_END) {
    ret = "stream end (Z_STREAM_END)";
  } else if (val == Z_NEED_DICT) {
    ret = "needs dictionary (Z_NEED_DICT)";
  } else if (val == Z_ERRNO) {
    ret = "(Z_ERRNO), check errno ";
  } else if (val == Z_STREAM_ERROR) {
    ret = "(Z_STREAM), stream error";
  } else if (val == Z_DATA_ERROR) {
    ret = "(Z_DATA), data error";
  } else if (val == Z_MEM_ERROR) {
    ret = "(Z_MEM), MEM error";
  } else if (val == Z_BUF_ERROR) {
    ret = "(Z_BUF), BUF error";
  } else if (val == Z_VERSION_ERROR) {
    ret = "(Z_VERSION), VERSION error";
  }
  return ret;
}
