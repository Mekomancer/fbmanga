
struct option{
  char opt;
  std::string long_opt;
  std::string desc;
  std::string long_desc;
};

int printHelp(std::string nullarg="");
int printVersion(std::string nullarg="");
std::array<option,2> opts({
    option{
      'h',
      "help",
      "Print help",
      "Prints help on invoking FBmanga",
    },
    option{
      'v',
      "version",
      "Print version",
      "Print the version of fbmanga installed",
    },      
});

