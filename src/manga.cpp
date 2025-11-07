#include "manga.h"

void mangadex::prepareRequest() {
  curl_easy_setopt(curl, CURLOPT_AUTOREFERER, true);
  curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, CURLFOLLOW_ALL);
  curl_easy_setopt(curl, CURLOPT_HSTS_CTRL, CURLHSTS_ENABLE);
  curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_LAST);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "FBManga/0.1");
  curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_3);
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
  prepareRequest();
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

int mangadex::init() { return (curl = curl_easy_init()) == NULL ? -1 : 0; }

std::vector<std::string> mangadex::getMangaId(std::string_view title) {
  prepareRequest();
  curl_easy_setopt(curl, CURLOPT_URL, (base_url + "/manga").c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fillbuf);
  std::vector<char> buffer;
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
  curl_easy_perform(curl);
  buffer.push_back('\000');
  dprf("{:s}", buffer.data());
  // params = {"title"= title}
  // response = curl get() or smth idk
  // return response.data("id") or smth idk
  return {""};
};

std::vector<std::string> mangadex::getChapterIds(std::string_view manga_id) {
  return {""};
}

std::vector<int> mangadex::downloadChapter(std::string_view chapter_id) {
  return {-1}; // dummy func, for now...
};
