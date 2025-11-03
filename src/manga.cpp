#include "manga.h"

int mangadex::setCreds(std::string name, std::string psswd, std::string id,
                       std::string secret) {
  username = name;
  password = psswd;
  client_id = id;
  client_secret = secret;
  return 0;
};

int mangadex::init() {
  return (curl_handle = curl_easy_init()) == NULL ? -1 : 0;
}

std::vector<std::string> mangadex::getMangaId(std::string title) {
  curl_easy_setopt(curl_handle, CURLOPT_URL, (base_url + "/manga").c_str());
  // params = {"title"= title}
  // response = curl get() or smth idk
  // return response.data("id") or smth idk
  return {""};
};

std::vector<std::string> mangadex::getChapterIds(std::string manga_id) {
  return {""};
}

std::vector<int> mangadex::downloadChapter(std::string chapter_id) {
  return {-1}; // dummy func, for now...
};
