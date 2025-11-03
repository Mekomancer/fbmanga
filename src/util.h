#pragma once

struct rgb888 {
  uint8_t red, green, blue;
};

template <std::integral T> /*png (network) to host byte order*/
[[nodiscard]] constexpr T ptoh(T val) noexcept {
  using enum std::endian;
  if (native == big) {
    return val;
  } else if (native == little) {
    return std::byteswap(val);
  } else {
    static_assert((native == little) || (native == big),
                  "Mixed-endian byte order not supported");
  }
}

template <std::integral T> /*host to png (network) byte order*/
[[nodiscard]] constexpr T htop(T val) noexcept {
  using enum std::endian;
  if (native == big) {
    return val;
  } else if (native == little) {
    return std::byteswap(val);
  } else {
    static_assert((native == little) || (native == big),
                  "Mixed-endian byte order not supported");
  }
}

template <std::unsigned_integral T>
constexpr T bitscale(T val, int cur, int target) {
  //  static_assert((val & ~((1<<cur)-1))==0, "Input bitdepth must be greater
  //  than or eqaul to than real bitdepth"); static_assert(~((T) 0) >=
  //  ((1<<cur)-1), "The number of bits in the input type must be greater than
  //  input bitdepth");
  return (((2 * val * ((1 << target) - 1)) / ((1 << cur) - 1)) + 1) / 2;
};

template <std::unsigned_integral uint_t>
constexpr uint_t bitscaletrue(uint_t val, int cur, int target) {
  return static_cast<uint_t>(
      (static_cast<double>(val) * static_cast<double>((1 << target) - 1) /
       static_cast<double>((1 << cur) - 1)) +
      0.5); /* implicit conversion truncs, which is okay because the value
             * will be non-negitive, so trunc(x) == floor(x). png spec states
             * the most accurate method for sample depth scaling is the linear
             * equation `floor((val* maxtargetsamp / maxcursamp)+0.5)` where
             * maxtargetsamp = 2^target - 1 and maxcursamp = 2^cur - 1
             * the reason for not using std::floor is that in llvm-19
             * it is not constexpr, which is useful to see if my function that
             * skips the conversion to doubles is accurate
             */
}

template <typename T> class ring_buf {
private:
  struct page_t {
    std::byte *addr;
    size_t size;
    bool in_use;
    page_t();
    ~page_t();
  };
  struct address_t {
    size_t chunk;
    size_t offset;
  };
  std::vector<page_t> chunks;
  address_t start = {0, 0};
  size_t len = 0;
  address_t end = {0, 0};
  address_t addrAt(size_t index);

public:
  void resize(size_t count);
  template <typename ret_t = T> ret_t pop();
  size_t mmove(std::span<std::byte> dest);
  size_t mcopy(std::span<std::byte> dest);
  T *data();
  void append(std::byte val);
  size_t size();
  size_t capacity();
};

template <typename T> template <typename ret_t> ret_t ring_buf<T>::pop() {
  ret_t val;
  if (sizeof(ret_t) + start.offset < chunks[start.chunk].size) {
    val = *reinterpret_cast<ret_t *>(&chunks[start.chunk].addr[start.offset]);
  } else {
    std::vector<std::byte> valarray;
    valarray.resize(sizeof(T));
    for (int i = 0; i < valarray.size(); i++) {
      address_t loc = addrAt(i);
      valarray[i] = chunks[loc.chunk].addr[loc.offset];
    }
    val = *reinterpret_cast<ret_t *>(valarray.data());
  }

  start = addrAt(sizeof(ret_t));
  len -= sizeof(ret_t);
  return val;
}

constexpr std::string colorTypeString(uint8_t val);

constexpr std::string zlib_return_string(int val);
