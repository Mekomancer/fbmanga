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
  curl_easy_setopt(curl, CURLOPT_URL, (base_url + "/manga").c_str());
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
