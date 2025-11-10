using namespace std::literals;
#pragma once
#ifdef NDEBUG
#define dprf(...)
#else
#define dprf(...)                                                              \
  std::print(__VA_ARGS__);                                                     \
  fflush(0);
#endif
struct rgb888 {
  uint8_t red, grn, blu;
};

/*png (network) to host byte order*/
template <std::integral T> [[nodiscard]] constexpr T ptoh(T val) noexcept {
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
template <std::integral T> [[nodiscard]] constexpr T htop(T val) noexcept {
  using enum std::endian;
  if constexpr (native == big) {
    return std::byteswap(val);
  } else if (native == little) {
    return val;
  } else {
    static_assert((native == little) || (native == big),
                  "Mixed-endian unsupported");
  }
}

template <std::unsigned_integral T>
constexpr T bitscale(T val, int cur, int target) {
  return (((2 * val * ((1 << target) - 1)) / ((1 << cur) - 1)) + 1) / 2;
};

template <std::unsigned_integral uint_t>
constexpr uint_t bitscaletrue(uint_t val, int cur, int target) {
  return static_cast<uint_t>(
      (static_cast<double>(val) * static_cast<double>((1 << target) - 1) /
       static_cast<double>((1 << cur) - 1)) +
      0.5);
  /* implicit conversion truncs, which is okay because the value
   * will be non-negitive, so trunc(x) == floor(x). png spec states
   * the most accurate method for sample depth scaling is the linear
   * equation `floor((val* maxtargetsamp / maxcursamp)+0.5)` where
   * maxtargetsamp = 2^target - 1 and maxcursamp = 2^cur - 1
   * the reason for not using std::floor is that in llvm-19
   * it is not constexpr, which is useful to see if my function that
   * skips the conversion to doubles is accurate
   */
}

template <typename t>
concept bytesized = requires() { new char[(sizeof(t) == 1 ? 0 : -1)]; };

class ring_buf {
public:
  template <bytesized t = uint8_t> size_t peek(std::span<t> buf);
  template <bytesized t = uint8_t> size_t pop(std::span<t> buf);
  template <bytesized t = uint8_t> size_t append(std::span<t> buf);
  size_t size();
  size_t len();
  size_t resize();
  bool auto_resize = true;
  bool constexpr is_wrapped();
  ring_buf();
  ~ring_buf();
  ring_buf &operator=(ring_buf &other) = delete;

private:
  std::vector<uint8_t> data;
  size_t begin;
  size_t end;
};

template <bytesized t> size_t ring_buf::peek(std::span<t> buf) {
  size_t num_bytes = std::min(buf.size(), len());
  if (!is_wrapped()) {
    buf.subspan(0, num_bytes) =
        std::span<t>(reinterpret_cast<t *>(&data[begin]), num_bytes);
  } else {
    size_t begin_bytes = std::min(num_bytes, (data.size() - (begin + 1)));
    buf.subspan(0, begin_bytes) =
        std::span<t>(reinterpret_cast<t *>(&data[begin]), begin_bytes);
    if (begin_bytes < num_bytes) {
      size_t end_bytes = std::min(num_bytes - begin_bytes, end);
      buf.subspan(begin_bytes, end_bytes) =
          std::span<t>(reinterpret_cast<t *>(&data[0]), end_bytes);
      if (end_bytes + begin_bytes != num_bytes) {
        dprf("ERR: ring_buf::peek(), mark couldn't count properly");
      }
    }
  }
  return num_bytes;
}

template <bytesized t> size_t ring_buf::pop(std::span<t> buf) {
  size_t ret = peek<t>(buf);
  begin = (begin + buf.size()) % data.size();
  return ret;
}

template <bytesized t> size_t ring_buf::append(std::span<t> buf) {
  size_t num_bytes = std::min(buf.size(), data.size() - len());
  if (end + num_bytes < data.size()) {
    std::memcpy(data.data(), buf.data(), num_bytes);
  } else {
    size_t begin_bytes = std::min(num_bytes, (data.size() - (end + 1)));
    if (begin_bytes > 0) {
      std::memcpy(&data[end], buf.data(), begin_bytes);
    }
    if (begin_bytes < num_bytes) {
      size_t end_bytes = std::min(num_bytes - begin_bytes, end);
      std::memcpy(data.data(), &buf[begin_bytes], end_bytes);
      if (end_bytes + begin_bytes != num_bytes) {
        dprf("ERR: ring_buf::peek(), mark couldn't count properly");
      }
    }
  }
  return num_bytes;
}

class configuration {
public:
  std::string mangadex_api_url = "api.mangadex."
#ifndef NDEBUG
                                 "dev";
#else
                                 "org";
#endif
  void indexArgs(int argn, char *argv[]);
  int parseArgs();

private:
  std::vector<std::string> args;
};

void printHelp();
void printVersion();
