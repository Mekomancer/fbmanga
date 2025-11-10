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
}

class ring_buf {
public:
  template <typename t = uint8_t> size_t peek(std::span<t> buf);
  template <typename t = uint8_t> size_t read(std::span<t> buf);
  template <typename t = uint8_t> t pop() {
    std::vector<uint8_t> buf(sizeof(t));
    read(std::span(buf));
    return *reinterpret_cast<t *>(buf.data());
  }
  template <typename t = uint8_t> size_t append(std::span<t> buf);
  size_t size();
  size_t len();
  size_t resize();
  bool constexpr is_wrapped() {
  if (end < begin) {
    return true;
  } else if (begin == end) {
    return false;
  } else if (end > begin) {
    return false;
  } else {
    return true;
  }
}
  ring_buf();
  ~ring_buf();
  ring_buf &operator=(ring_buf &other) = delete;

private:
  std::vector<uint8_t> data;
  size_t begin = 0;
  size_t end = 0;
};

template <typename t> size_t ring_buf::peek(std::span<t> bufffer) {
  std::span<std::byte> buf = std::as_writable_bytes(bufffer);
  size_t num_bytes = std::min(buf.size(), len());
  if (!is_wrapped()) {
    buf.subspan(0, num_bytes) = std::span<std::byte>(
        reinterpret_cast<std::byte *>(&data[begin]), num_bytes);
  } else {
    size_t begin_bytes = std::min(num_bytes, (data.size() - (begin + 1)));
    buf.subspan(0, begin_bytes) = std::span<std::byte>(
        reinterpret_cast<std::byte *>(&data[begin]), begin_bytes);
    if (begin_bytes < num_bytes) {
      size_t end_bytes = std::min(num_bytes - begin_bytes, end);
      buf.subspan(begin_bytes, end_bytes) = std::span<std::byte>(
          reinterpret_cast<std::byte *>(&data[0]), end_bytes);
      if (end_bytes + begin_bytes != num_bytes) {
        dprf("ERR: ring_buf::peek(), mark couldn't count properly");
      }
    }
  }
  return num_bytes;
}

template <typename t> size_t ring_buf::read(std::span<t> buf) {
  size_t ret = peek<t>(buf);
  begin = (begin + buf.size_bytes()) % data.size();
  return ret;
}

template <typename t> size_t ring_buf::append(std::span<t> buf) {
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
  end = (end + num_bytes) % data.size();
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
