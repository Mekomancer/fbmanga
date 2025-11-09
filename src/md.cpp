#include "md.h"
//#include "util.h"
namespace md {
void MangaList::fill(std::string_view str) {
  std::cmatch part;
  std::regex reg;
  reg = "^\\{\"result\":\"((?:[^\"]|\\\")*)\","
        "\"response\":\"((?:[^\"]|\\\")*)\","
        "\"data\":\\\[(.*)\\],"
	"\"limit\":(\\d*),"
	"\"offset\":(\\d*),"
	"\"total\":(\\d)\\}$";
  std::regex_match(str.data(), part, reg);
  result = part[1].str();
  response = part[2].str();
  limit = std::stoi(part[4].str());
  offset = std::stoi(part[5].str());
  total = std::stoi(part[6].str());
   
};
}; // namespace md
