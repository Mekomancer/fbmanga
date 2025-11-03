#include "manga.h"

int mangadex::setCreds(std::string_view name, std::string_view psswd,
                       std::string_view id, std::string_view secret) {
  username = name;
  password = psswd;
  client_id = id;
  client_secret = secret;
  return 0;
};

std::vector<std::string> mangadex::getImgUrls(std::string_view chapter) {
  return {""};
}

int mangadex::init() {
  return (curl_handle = curl_easy_init()) == NULL ? -1 : 0;
}

std::vector<std::string> mangadex::getMangaId(std::string_view title) {
  curl_easy_setopt(curl_handle, CURLOPT_URL, (base_url + "/manga").c_str());
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
