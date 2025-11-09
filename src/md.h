namespace md {
/*--------Enums--------*/

enum struct demographic {
  shounen,
  shoujo,
  josei,
  seinen,
};
enum struct status {
  ongoing,
  completed,
  hiatus,
  cancelled,
};

enum struct reading_status {
  reading,
  on_hold,
  plan_to_read,
  dropped,
  re_reading,
  completed,
};

enum struct rating {
  safe,
  suggestive,
  erotica,
  pornographic,
};

/*-------Objects-------*/
typedef std::string uuid;
const std::string uuidregex =
    "[:xdigit:]{8}-[:xdigit:]{4}-[:xdigit:]{4}-[:xdigit:]{4}-[:xdigit:]{12}";
struct LocalizedString { /*"en":"Place to Place" or "ja-ro":"Acchi Kocchi"*/
  std::string lang;
  std::wstring data;
  void fill(std::string str);
};

struct MangaAttributes {
  LocalizedString title;
  std::vector<LocalizedString> altTitles;
  LocalizedString description;
  bool isLocked;
  std::map<std::string, std::string> links; /* key, url pairs*/
  std::string originalLanguage;
  std::string lastVolume;
  std::string last_chapter;
  void fill(std::string str);
};

struct Relationship {
  uuid id;
  std::string type;
  std::string related;
  void fill(std::string str);
};

struct Manga {
  std::string id;
  std::string type;
  MangaAttributes attributes;
  std::vector<Relationship> relationships;
  void fill(std::string str);
};

struct MangaList {
  std::string result;
  std::string response;
  std::vector<Manga> data;
  int limit;
  int offset;
  int total;
  void fill(std::string_view str);
};
}; // namespace md
