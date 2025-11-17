export module debug;
import std;
export import config;

export template <typename... args_t>
void dprf(configuration::verboseness severity, std::format_string<args_t...> fmt, args_t &&...args) {
  if (conf.verbosity >= severity) {
    std::string prefix = "";
    switch (severity) {
      using enum configuration::verboseness;
    case err:
      prefix = "!ERR: ";
      break;
    case warn:
      prefix = "WARN: ";
      break;
    case info:
      prefix = "INFO: ";
      break;
    case dump:
      prefix = "DUMP: ";
      break;
    default:
      prefix = "MISC: ";
      break;
    }
    std::print("{}",prefix);
    std::print(fmt, std::forward<args_t>(args)...);
    std::fflush(0);
  }
}

export template <typename... args_t>
void err(std::format_string<args_t...> fmt, args_t &&...args) {
  dprf(configuration::verboseness::err,fmt,std::forward<args_t>(args)...);
}
export template <typename... args_t>
void warn(std::format_string<args_t...> fmt, args_t &&...args) {
  dprf(configuration::verboseness::warn,fmt,std::forward<args_t>(args)...);
}
export template <typename... args_t>
void info(std::format_string<args_t...> fmt, args_t &&...args) {
  dprf(configuration::verboseness::info,fmt,std::forward<args_t>(args)...);
}
export template <typename... args_t>
void dump(std::format_string<args_t...> fmt, args_t &&...args) {
  dprf(configuration::verboseness::dump,fmt,std::forward<args_t>(args)...);
}
