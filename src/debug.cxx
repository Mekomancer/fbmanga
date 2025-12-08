export module debug;
import std;
import config;
import types;

export std::source_location
here(std::source_location loc = std::source_location::current()) {
  return loc;
}

std::string current_file = "";
std::string current_func = "";

export template <typename... args_t>
void log_raw(std::format_string<args_t...> fmt, args_t &&...args) {
  std::print(conf.log, fmt, args...);
}
export template <typename... args_t>
void log(std::format_string<args_t...> fmt, std::source_location loc,
         args_t &&...args) {
  if (!conf.logging["all"]) {
    for (auto msg_type : conf.logging) {
      if (fmt.get().starts_with(msg_type.first + ":") &&
          msg_type.second == false) {
        return;
      }
    }
  }
  if (conf.logging["location"]) {
    if (loc.file_name() != current_file) {
      std::print(conf.log, "{}\n", loc.file_name());
      current_file = loc.file_name();
    }
    std::print("|");
    if (loc.function_name() != current_func) {
      std::print(conf.log, "{}\n|", loc.function_name());
      current_func = loc.function_name();
    }
    std::print("|");
    std::print("{:4} ", loc.line());
  }
  std::print(conf.log, fmt, std::forward<args_t>(args)...);
#ifndef NDEBUG
  std::fflush(conf.log);
#endif
}
