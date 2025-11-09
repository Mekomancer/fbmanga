#include "manga.h"
#include "util.h"

extern configuration conf;

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
size_t fillbuf(char *ptr, size_t size, size_t nmemb, void *userdata) {
  std::vector<uint8_t> *buf =
      reinterpret_cast<std::vector<uint8_t> *>(userdata);
  if (size != 1) {
    dprf("WARN: size ({:}) != 1, strange...\n", size);
  }
  buf->resize(buf->size() + nmemb);
  std::memcpy(buf->data() + buf->size() - nmemb, ptr, nmemb);
  return nmemb;
}
// ditto
size_t fillstr(char *ptr, size_t size, size_t nmemb, void *userdata) {
  if (size != 1) {
    dprf("WARN: size ({:}) != 1, strange...\n", size);
  }
  reinterpret_cast<std::string *>(userdata)->append(ptr, nmemb);
  return nmemb;
};

bool mangadex::checkup() {
  prepareCurl();
  setEndpoint("get-ping");
  std::vector<char> buffer;
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fillbuf);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
  CURLcode ret = curl_easy_perform(curl);
  buffer.push_back('\000');
  if (std::format("{:s}", buffer.data()) == "pong") {
    return true;
  } else {
    dprf("ERR: MangaDex's ping healthcheck failed, curl returned {:}({:})",
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

std::vector<std::string> getScanlationGroups(std::string chap) { return {""}; }
std::vector<std::string> mangadex::getImgUrls(std::string_view chapter) {
  return {""};
}

mangadex::mangadex() {
  curl = curl_easy_init();
  url = curl_url();
}
void mangadex::setEndpoint(std::string_view endp) {
  if (endp == "get-search-manga") {
    curl_url_set(url, CURLUPART_PATH, "manga", 0);
  } else if (endp == "get-ping") {
    curl_url_set(url, CURLUPART_PATH, "ping", 0);
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
  prepareCurl();
  setEndpoint("get-search-manga");
  clearQuery();
  queryAdd("title", title);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fillstr);
  std::string buffer;
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
  curl_easy_perform(curl);
  md::MangaList ml;
  ml.fill(buffer);
  return {""};
};

std::vector<std::string> mangadex::getChapterIds(std::string_view manga_id) {
  return {""};
}

std::vector<int> mangadex::downloadChapter(std::string_view chapter_id) {
  return {-1};
};
