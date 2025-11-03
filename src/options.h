
struct option {
  char opt;
  std::string long_opt;
  std::function<int(std::string_view)> func;
  std::string desc;
  std::string long_desc;
};

int printHelp(std::string_view nullarg = "");
int printVersion(std::string_view nullarg = "");
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
