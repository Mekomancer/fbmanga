
struct option {
  char opt;
  std::string long_opt;
  std::function<int(std::string)> func;
  std::string desc;
  std::string long_desc;
};

int printHelp(std::string nullarg = "");
int printVersion(std::string nullarg = "");
std::array<option, 2> opts({
    option{
        'h',
        "help",
        printHelp,
        "Print help",
        "Prints help on invoking FBmanga",
    },
    option{
        'v',
        "version",
        printVersion,
        "Print version",
        "Print the version of fbmanga installed",
    },
});
