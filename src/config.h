
class configuration {
private:
  std::vector<std::string> args;
  std::string mangadex_api_url = "api.mangadex."
#ifndef NDEBUG
                                 "dev";
#else
                                 "org";
#endif
public:
  void indexArgs(int argn, char *argv[]);
  int parseArgs();
};

void printHelp();
void printVersion();
