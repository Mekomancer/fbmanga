module;
#include <unistd.h>
export module png.util;
import std;
import types;
import std.compat;
export class ring_buf {
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
  size_t resize(size_t);
  bool constexpr is_wrapped() {
    if (end < begin) {
      return true;
    } else {
      return false;
    }
  }
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
    std::memcpy(buf.data(), &data[begin], num_bytes);
  } else {
    size_t begin_bytes = std::min(num_bytes, (data.size() - (begin + 1)));
    buf.subspan(0, begin_bytes) = std::span<std::byte>(
        reinterpret_cast<std::byte *>(&data[begin]), begin_bytes);
    if (begin_bytes < num_bytes) {
      size_t end_bytes = std::min(num_bytes - begin_bytes, end);
      buf.subspan(begin_bytes, end_bytes) = std::span<std::byte>(
          reinterpret_cast<std::byte *>(&data[0]), end_bytes);
      if (end_bytes + begin_bytes != num_bytes) {
        dprf("ERR: ring_buf::peek()");
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
  size_t num_bytes = buf.size();
  if (len() + num_bytes > size()) {
    resize(len() + num_bytes);
  }
  if (end + num_bytes < data.size()) {
    std::memcpy(&data[end], buf.data(), num_bytes);
  } else {
    size_t begin_bytes = std::min(num_bytes, (data.size() - (end + 1)));
    if (begin_bytes > 0) {
      std::memcpy(&data[end], buf.data(), begin_bytes);
    }
    if (begin_bytes < num_bytes) {
      size_t end_bytes = std::min(num_bytes - begin_bytes, end);
      std::memcpy(data.data(), &buf[begin_bytes], end_bytes);
      if (end_bytes + begin_bytes != num_bytes) {
        dprf("ERR: ring_buf::peek()");
      }
    }
  }
  end = (end + num_bytes) % data.size();
  return num_bytes;
}
size_t ring_buf::resize(size_t newlen) {
  if (newlen < data.size()) {
    return data.size();
  };
  size_t oldsize = data.size();
  data.resize(newlen + getpagesize());
  if (is_wrapped()) {
    std::memcpy(&data[oldsize], data.data(), end);
    end = oldsize + end;
  }
  return data.size();
}

size_t ring_buf::size() { return data.size(); }

size_t ring_buf::len() {
  if (is_wrapped()) {
    return end + (data.size() - (begin + 1));
  } else {
    return end - begin;
  }
}
