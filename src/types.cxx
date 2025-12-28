module;
#include <cstdint>
#include <curses.h>
export module types;
import std;
export using namespace std::placeholders;
export typedef std::uint8_t uint8_t;
export typedef std::uint16_t uint16_t;
export typedef std::uint32_t uint32_t;
export typedef std::uint64_t uint64_t;
export typedef std::size_t size_t;
export struct rgb888 {
  uint8_t red, grn, blu;
};
export template <typename T, typename E> using Result = std::expected<T, E>;
export template <typename E> using Err = std::unexpected<E>;
export template <typename T> using Option = std::optional<T>;
export template <typename... Types> using Enum = std::variant<Types...>;
export template <typename T> using Vec = std::vector<T>;
export template <typename R, typename... Args>
using fn = std::function<R(Args...)>;
export const std::nullopt_t None = std::nullopt;
export typedef std::string String;
export typedef std::string_view StringView;

constexpr int _KEY_UP = KEY_UP;
#undef KEY_UP
constexpr int KEY_UP = _KEY_UP;
constexpr int _KEY_DOWN = KEY_DOWN;
#undef KEY_DOWN
constexpr int KEY_DOWN = _KEY_DOWN;
constexpr int _KEY_RIGHT = KEY_RIGHT;
#undef KEY_RIGHT
constexpr int KEY_RIGHT = _KEY_RIGHT;
constexpr int _KEY_LEFT = KEY_LEFT;
#undef KEY_LEFT
constexpr int KEY_LEFT = _KEY_LEFT;
