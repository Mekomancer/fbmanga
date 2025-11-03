
class configuration {
private:
  std::vector<std::string> args;

public:
  void indexArgs(int argn, char *argv[]);
  int parseArgs();
};
