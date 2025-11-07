#include "manga.h"

void mangadex::prepareCurl() {
  curl_easy_setopt(curl, CURLOPT_AUTOREFERER, true);
  curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, CURLFOLLOW_ALL);
  curl_easy_setopt(curl, CURLOPT_HSTS_CTRL, CURLHSTS_ENABLE);
  curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_LAST);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "FBManga/0.1");
  curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_3);
  curl_easy_setopt(curl, CURLOPT_NOPROGRESS, !false);
  curl_url_set(url, CURLUPART_SCHEME, "https://", 0);
  curl_url_set(url, CURLUPART_HOST, base_url.data(), 0);
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

bool mangadex::checkup() {
  prepareCurl();
  curl_easy_setopt(curl, CURLOPT_URL, (base_url + "/ping").c_str());
  dprf("ping..");
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fillbuf);
  std::vector<char> buffer;
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
  CURLcode ret = curl_easy_perform(curl);
  buffer.push_back('\000');
  dprf("{:s}\n", buffer.data());
  if (std::format("{:s}", buffer.data()) == "pong") {
    return true;
  } else {
    dprf("ERR: MangaDex's ping healthcheck failed");
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

int mangadex::init() {
  curl = curl_easy_init();
  url = curl_url();
  return 0;
}
void mangadex::setEndpoint(std::string_view endp) {
  if (endp == "get-search-manga") {
    curl_url_set(url, CURLUPART_PATH, "manga", 0);
  }
  return;
}
std::vector<std::string> mangadex::getMangaId(std::string_view title) {
  prepareCurl();
  setEndpoint("get-search-manga");
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fillbuf);
  std::vector<char> buffer;
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
  curl_easy_perform(curl);
  buffer.push_back('\000');
  dprf("{:s}", buffer.data());
  return {""};
};

std::vector<std::string> mangadex::getChapterIds(std::string_view manga_id) {
  return {""};
}

std::vector<int> mangadex::downloadChapter(std::string_view chapter_id) {
  return {-1};
};
