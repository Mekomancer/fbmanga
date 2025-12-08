module;
#include <curl/curl.h>
#include <rapidjson/document.h>
export module manga.dex;
import config;
import std;
import debug;
import util;

using namespace std::literals;
namespace rj = rapidjson;
export class mangadex {
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
  int downloadImg(std::string_view img_url, ring_buf *buf);
  int initTokens();
  std::string getAccessToken();
  static constexpr time_t access_token_lifetime = 60 * 15; // as of 09/03/25
private:
  bool report();
  int http_status;
  void dumpUrl();
  CURLcode perform(std::source_location loc = std::source_location::current());
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

bool mangadex::report() {
  char *img_url = nullptr;
  bool ok = false;
  curl_off_t size;
  curl_off_t dur;
  curl_header *xcache = nullptr;
  bool cache = false;
  if (curl_easy_header(curl, "X-Cache", 0, CURLH_HEADER, -1, &xcache) ==
      CURLHE_OK) {
    log("dump:{}: {}\n", here(), xcache->name, xcache->value);
    if (std::strcmp(xcache->value, "HIT") != 0) {
      cache = true;
    }
  };
  curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &size);
  if (size == -1) {
    size = 0;
  }
  long code;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
  if (code < 300 && code >= 200) {
    ok = true;
  }
  curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME_T, &dur);
  curl_url_get(url, CURLUPART_URL, &img_url, 0);
  prepareCurl();
  curl_url_set(url, CURLUPART_HOST, "api.mangadex.network", 0);
  curl_mime *mime = curl_mime_init(curl);
  curl_mimepart *mcontent = curl_mime_addpart(mime);
  std::string mdata =
      std::format("{{\"url\":\"{}\",\"success\":{:s},\"bytes\":{},"
                  "\"duration\":{},\"cached\":{:s}}}",
                  img_url, ok, size, dur / 1'000, cache);
  log("dump: /report data: {}\n", here(), mdata);
  log("info: status code: {}\n", here(), code);
  curl_mime_type(mcontent, "application/json");
  curl_mime_data(mcontent, mdata.c_str(), mdata.length());
  curl_easy_perform(curl);
  curl_free(img_url);
  return ok;
}

// curl callback func
size_t fillstr(char *ptr, size_t size, size_t nmemb, void *userdata) {
  if (size != 1) {
    log("warn:size ({:}) != 1, strange...\n", here(), size);
  }
  reinterpret_cast<std::string *>(userdata)->append(ptr, nmemb);
  return nmemb;
};

size_t fillbuf(char *ptr, size_t size, size_t nmemb, void *userdata) {
  if (size != 1) {
    log("warn:size ({:}) != 1, strange...\n", here(), size);
  }
  reinterpret_cast<ring_buf *>(userdata)->append(std::span(ptr, nmemb));
  return nmemb;
}
void mangadex::prepareCurl() {
  curl_easy_reset(curl);
  curl_easy_setopt(curl, CURLOPT_AUTOREFERER, true);
  curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, CURLFOLLOW_ALL);
  curl_easy_setopt(curl, CURLOPT_HSTS_CTRL, CURLHSTS_ENABLE);
  curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_LAST);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "FBManga/0.1");
  curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_3);
  curl_easy_setopt(curl, CURLOPT_NOPROGRESS, !false);
  curl_url_cleanup(url);
  url = curl_url();
  curl_easy_setopt(curl, CURLOPT_CURLU, url);
  curl_url_set(url, CURLUPART_SCHEME, "https", 0);
  curl_url_set(url, CURLUPART_HOST, conf.mangadex_api_url.c_str(), 0);
};

CURLcode mangadex::perform(std::source_location loc) {
  log("jrnl: Starting transfer...", loc);
  CURLcode ret = curl_easy_perform(curl);
  log_raw("done\n");
  long httpcode = -1;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpcode);
  log("info: curl returned `{}` ({}), http code: {} \n", loc,
      static_cast<int>(ret), curl_easy_strerror(ret), httpcode);
  curl_off_t ttfb;
  curl_easy_getinfo(curl, CURLINFO_STARTTRANSFER_TIME_T, &ttfb);
  curl_off_t total;
  curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME_T, &total);
  log("stat: TTFB: {}ms, DOWNLOAD: {}ms, TOTAL {}ms\n", loc, ttfb / 1000,
      (total - ttfb) / 1000, total / 1000);
  return ret;
}

bool mangadex::checkup() {
  log("jrnl: Starting MangaDex healthcheck\n", here());
  prepareCurl();
  setEndpoint("get-ping");
  std::string buffer;
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fillstr);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
  CURLcode ret = perform();
  log("dump: {}\n", here(), buffer);
  if (ret == CURLE_OK && buffer == "pong") {
    return true;
  } else {
    log("error: ", here());
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

int mangadex::downloadImg(std::string_view img_url, ring_buf *buf) {
  log("dump: image url: {}\n", here(), img_url);
  log("info: Downloading image...", here());
  prepareCurl();
  curl_url_set(url, CURLUPART_URL, img_url.data(), 0);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fillbuf);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, buf);
  CURLcode code = perform();
  log("info: Finished downloading image\n", here());
  bool ok = report();
  if (code == CURLE_OK) {
    return ok ? 0 : -1;
  } else {
    log("error: curl returned {} ({})\n", here(), static_cast<int>(code),
        curl_easy_strerror(code));
    return -1;
  }
}

std::vector<std::string> mangadex::getImgUrls(std::string_view chapter) {
  log("dump: Chapter UUID: {}", here(), chapter);
  log("jrnl: Retriving image urls...", here());
  prepareCurl();
  setEndpoint("get-at-home-server-chapterId", chapter);
  clearQuery();
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fillstr);
  std::string buffer;
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
  CURLcode code = perform();
  log("dump: MD returned \"{:}\"\n", here(), buffer);
  if (code != CURLE_OK) {
    log("error: curl returned {} ({})\n", here(), static_cast<int>(code),
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
  log("jrnl: Retrived image urls successfully\n", here());
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
  log("info:Searching for manga with title {:}\n", here(), title);
  prepareCurl();
  setEndpoint("get-search-manga");
  clearQuery();
  queryAdd("title", title);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fillstr);
  std::string buffer;
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
  CURLcode code = perform();
  log("info:MD returned \"{:}\"\n", here(), buffer);
  if (code != CURLE_OK) {
    log("error: curl returned {} ({})\n", here(), static_cast<int>(code),
        curl_easy_strerror(code));
    return {};
  }
  rj::Document doc;
  doc.Parse(buffer.c_str());
  std::vector<std::string> ret(doc["data"].GetArray().Size());
  std::transform(
      doc["data"].GetArray().Begin(), doc["data"].GetArray().End(), ret.begin(),
      [](rj::Value &val) { return std::string(val["id"].GetString()); });
  log("info: Found manga successfully\n", here());
  log("dump: Manga UUIDs {}\n", here(), ret);
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
  CURLcode code = perform();
  if (code != CURLE_OK) {
    log("error: curl returned {} ({})\n", here(), static_cast<int>(code),
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
  char *str;
  curl_url_get(url, CURLUPART_URL, &str, 0);
  log("dump:{}\n", here(), str);
  curl_free(str);
};
