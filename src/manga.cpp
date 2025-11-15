import std;
import png:util;
class mangadex {
public:
  mangadex();
  bool checkup(); // returns true if the ping endpoint works
  int setCreds(std::string_view username, std::string_view password,
               std::string_view id, std::string_view secret);
  struct chapter_info {
    std::string id;
    std::string desc;
  };
  std::vector<chapter_info> getChapters(std::string_view manga_id);
  std::vector<std::string> getMangaId(std::string_view title = "acchi kocchi");
  std::vector<std::string> getImgUrls(std::string_view chapter);
  void downloadImg(std::string_view img_url, ring_buf *buf);
  int initTokens();
  std::string getAccessToken();
  static constexpr time_t access_token_lifetime = 60 * 15; // as of 09/03/25
private:
  void dumpUrl();
  void queryAdd(std::string_view param, std::string_view val);
  void clearQuery();
  void setEndpoint(std::string_view endpoint);
  void setEndpoint(std::string_view endpoint, std::string_view value);
  std::string getGroupName(std::string_view groupId);
  CURLU *url;
  std::string username;
  std::string password;
  std::string client_id;
  std::string client_secret;
  std::string access_token;
  std::string refresh_token;
  time_t access_token_expiration_date;
  CURL *curl;
  void prepareCurl();
};
namespace rj = rapidjson;

void mangadex::prepareCurl() {
  curl_easy_setopt(curl, CURLOPT_AUTOREFERER, true);
  curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, CURLFOLLOW_ALL);
  curl_easy_setopt(curl, CURLOPT_HSTS_CTRL, CURLHSTS_ENABLE);
  curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_LAST);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "FBManga/0.1");
  curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_3);
  curl_easy_setopt(curl, CURLOPT_NOPROGRESS, !false);
  curl_easy_setopt(curl, CURLOPT_CURLU, url);
  curl_url_set(url, CURLUPART_SCHEME, "https", 0);
  curl_url_set(url, CURLUPART_HOST, conf.mangadex_api_url.c_str(), 0);
};

// curl callback func
size_t fillstr(char *ptr, size_t size, size_t nmemb, void *userdata) {
  if (size != 1) {
    warn("size ({:}) != 1, strange...\n", size);
  }
  reinterpret_cast<std::string *>(userdata)->append(ptr, nmemb);
  return nmemb;
};

size_t fillbuf(char *ptr, size_t size, size_t nmemb, void *userdata) {
  if (size != 1) {
    warn("size ({:}) != 1, strange...\n", size);
  }
  reinterpret_cast<ring_buf *>(userdata)->append(std::span(ptr, nmemb));
  return nmemb;
}

bool mangadex::checkup() {
  info("Starting MangaDex ping healthcheck\n");
  prepareCurl();
  setEndpoint("get-ping");
  std::string buffer;
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fillstr);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
  CURLcode ret = curl_easy_perform(curl);
  dump("/ping returned \"{:}\"", buffer);
  if (ret == CURLE_OK && buffer == "pong") {
    info("MangaDex's ping healthcheck succeeded\n");
    return true;
  } else {
    err("MangaDex's ping healthcheck failed, curl returned {:} ({:})\n",
        static_cast<int>(ret), curl_easy_strerror(ret));
    return false;
  }
}

int mangadex::setCreds(std::string_view name, std::string_view psswd,
                       std::string_view id, std::string_view secret) {
  username = name;
  password = psswd;
  client_id = id;
  client_secret = secret;
  return 0;
};

void mangadex::downloadImg(std::string_view img_url, ring_buf *buf) {
  info("Downloading image from {:}\n", img_url);
  prepareCurl();
  curl_url_set(url, CURLUPART_URL, img_url.data(), 0);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fillbuf);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, buf);
  CURLcode code = curl_easy_perform(curl);
  if (code == CURLE_OK)
    info("Finished downloading image");
  if (code != CURLE_OK) {
    dprf("ERR: curl returned {} ({})", static_cast<int>(code),
         curl_easy_strerror(code));
  }
}

std::vector<std::string> mangadex::getImgUrls(std::string_view chapter) {
  info("Retriving image urls for chapter {:}", chapter);
  prepareCurl();
  setEndpoint("get-at-home-server-chapterId", chapter);
  clearQuery();
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fillstr);
  std::string buffer;
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
  CURLcode code = curl_easy_perform(curl);
  dump("MD returned \"{:}\"", buffer);
  if (code != CURLE_OK) {
    dprf("ERR: curl returned {} ({})", static_cast<int>(code),
         curl_easy_strerror(code));
    return {};
  }
  rj::Document doc;
  doc.Parse(buffer.c_str());
  std::vector<std::string> ret(doc["chapter"]["data"].GetArray().Size());
  std::string base = doc["baseUrl"].GetString();
  std::string hash = doc["chapter"]["hash"].GetString();
  std::transform(doc["chapter"]["data"].GetArray().Begin(),
                 doc["chapter"]["data"].GetArray().End(), ret.begin(),
                 [base, hash](rj::Value &val) {
                   return base + "/data/" + hash + "/" + val.GetString();
                 });
  info("Retrived image urls successfully");
  return ret;
}

mangadex::mangadex() {
  curl = curl_easy_init();
  url = curl_url();
  access_token_expiration_date = 0;
}
void mangadex::setEndpoint(std::string_view endp) {
  if (endp == "get-search-manga") {
    curl_url_set(url, CURLUPART_PATH, "manga", 0);
  } else if (endp == "get-ping") {
    curl_url_set(url, CURLUPART_PATH, "ping", 0);
  }
  return;
}
void mangadex::setEndpoint(std::string_view endp, std::string_view val) {
  if (endp == "get-manga-id-feed") {
    curl_url_set(url, CURLUPART_PATH, ("manga/"s + val + "/feed"s).c_str(), 0);
  } else if (endp == "get-at-home-server-chapterId") {
    curl_url_set(url, CURLUPART_PATH, ("at-home/server/"s + val).c_str(), 0);
  } else if (endp == "get-group-id") {
    curl_url_set(url, CURLUPART_PATH, ("group/"s + val).c_str(), 0);
  }
  return;
}
// void mangadex::client_secret
void mangadex::queryAdd(std::string_view param, std::string_view val) {
  curl_url_set(url, CURLUPART_QUERY, (param + "="s + val).c_str(),
               CURLU_APPENDQUERY | CURLU_URLENCODE);
};
void mangadex::clearQuery() { curl_url_set(url, CURLUPART_QUERY, "", 0); }
std::vector<std::string> mangadex::getMangaId(std::string_view title) {
  info("Searching for manga with title {:}\n", title);
  prepareCurl();
  setEndpoint("get-search-manga");
  clearQuery();
  queryAdd("title", title);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fillstr);
  std::string buffer;
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
  CURLcode code = curl_easy_perform(curl);
  info("MD returned \"{:}\"\n", buffer);
  if (code != CURLE_OK) {
    dprf("ERR: curl returned {} ({})\n", static_cast<int>(code),
         curl_easy_strerror(code));
    return {};
  }
  rj::Document doc;
  doc.Parse(buffer.c_str());
  std::vector<std::string> ret(doc["data"].GetArray().Size());
  std::transform(
      doc["data"].GetArray().Begin(), doc["data"].GetArray().End(), ret.begin(),
      [](rj::Value &val) { return std::string(val["id"].GetString()); });
  info("Found manga successfully\n");
  return ret;
};
std::vector<mangadex::chapter_info>
mangadex::getChapters(std::string_view manga_id) {
  prepareCurl();
  setEndpoint("get-manga-id-feed", manga_id);
  clearQuery();
  queryAdd("includes[]", "scanlation_group");
  queryAdd("translatedLanguage[]", "en");
  dumpUrl();
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fillstr);
  std::string buffer;
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
  CURLcode code = curl_easy_perform(curl);
  if (code != CURLE_OK) {
    dprf("ERR: curl returned {} ({})", static_cast<int>(code),
         curl_easy_strerror(code));
  }
  rj::Document doc;
  doc.Parse(buffer.c_str());
  std::vector<mangadex::chapter_info> ret;
  for (rj::Value &val : doc["data"].GetArray()) {
    std::string id = val["id"].GetString();
    rj::Value &attr = val["attributes"];
    std::string desc;
    if (!attr["chapter"].IsNull()) {
      desc += attr["chapter"].GetString() + " "s;
    }
    if (!attr["volume"].IsNull()) {
      desc += "vol "s + attr["volume"].GetString() + " "s;
    }
    if (!attr["title"].IsNull()) {
      desc += attr["title"].GetString();
      desc += " ";
    }
    for (rj::Value &rel : val["relationships"].GetArray()) {
      if (std::string(rel["type"].GetString()) == "scanlation_group") {
        desc += rel["attributes"]["name"].GetString() + " "s;
      }
    };
    ret.emplace_back(id, desc);
  }
  return ret;
}

void mangadex::dumpUrl() {
  if (conf.vebosity >= configuration::verboseness::dump) {
    char *str;
    curl_url_get(url, CURLUPART_URL, &str, 0);
    dump("{}", str);
  }
};
