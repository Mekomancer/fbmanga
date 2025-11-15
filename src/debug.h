#pragma once
import config;
#define dprf(...)                                                              \
  std::print(__VA_ARGS__);                                                     \
  std::fflush(0);
#define err(...)                                                               \
  if (conf.vebosity >= configuration::verboseness::error) {                      \
    dprf("!ERR:" __VA_ARGS__);                                                  \
  }
#define warn(...)                                                              \
  if (conf.vebosity >= configuration::verboseness::warn) {                     \
    dprf("WARN:" __VA_ARGS__);                                                  \
  }
#define info(...)                                                              \
  if (conf.vebosity >= configuration::verboseness::info) {                     \
    dprf("INFO:" __VA_ARGS__);                                                  \
  }
#define dump(...)                                                              \
  if (conf.vebosity >= configuration::verboseness::dump) {                     \
    dprf("INFO:" __VA_ARGS__);                                                  \
  }
