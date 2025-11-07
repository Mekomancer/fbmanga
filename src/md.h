namespace md {
/*--------Enums--------*/

namespace demographic {
constexpr std::string shounen = "shounen";
constexpr std::string shoujo = "shounjo";
constexpr std::string josei = "josei";
constexpr std::string seinen = "seinen";
} // namespace demographic
namespace status {
constexpr std::string ongoing = "ongoing";
constexpr std::string completed = "completed";
constexpr std::string hiatus = "hiatus";
constexpr std::string cancelled = "cancelled";
} // namespace status

namespace reading_status{
  constexpr std::string reading = "ongoing";
  constexpr std::string on_hold = "ongoing";
  constexpr std::string plan_to_read = "ongoing";
  constexpr std::string dropped = "ongoing";
  constexpr std::string re_reading = "ongoing";
  constexpr std::string completed = "ongoing";
}

namespace content_rating{
  constexpr std::string safe = "ongoing";
  constexpr std::string suggestive = "ongoing";
  constexpr std::string erotica = "ongoing";
  constexpr std::string pornographic = "ongoing";
}

typedef std::string uuid;

struct LocalizedString {
  std::string lang;
  std::wstring data;
};

struct MangaAttributes {
  LocalizedString title;
  std::vector<LocalizedString> altTitles;
  LocalizedString description;
  bool isLocked;
  // links
  std::string originalLanguage;
  std::string lastVolume;
  std::string last_chapter;
};

struct Relationship {
  uuid id;
  std::string type;
  std::string related;
};

struct Manga {
  std::string id;
  std::string type;
  MangaAttributes attributes;
  std::vector<Relationship> relationships;
};

struct MangaList {
  std::string result;
  std::string response;
  std::vector<Manga> data;
  int limit;
  int offset;
  int total;
};
}; // namespace md
