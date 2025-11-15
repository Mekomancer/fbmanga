#define dprf(...)                                                              \
  std::print(__VA_ARGS__);                                                     \
  fflush(0);
#define err(...)                                                               \
  if (conf::vebosity >= cofiguration::verboseness::err) {                      \
    dprf("!ERR:"__VA_ARGS__);                                                  \
  }
#define warn(...)                                                              \
  if (conf::vebosity >= cofiguration::verboseness::warn) {                     \
    dprf("WARN:"__VA_ARGS__);                                                  \
  }
#define info(...)                                                              \
  if (conf::vebosity >= cofiguration::verboseness::info) {                     \
    dprf("INFO:"__VA_ARGS__);                                                  \
  }
#define dump(...)                                                              \
  if (conf::vebosity >= cofiguration::verboseness::dump) {                     \
    dprf("INFO:"__VA_ARGS__);                                                  \
  }
