#include "pch.h"
#include "mangadex.h"


int mangadex_auth::setCreds(std::string name, std::string psswd, std::string id, std::string secret){
  username = name;
  password = psswd;
  client_id = id;
  client_secret = secret;
  return 0;
};

mangadex_auth::mangadex_auth(){
  curl_handle = curl_easy_init();
};

int mangadex_manga::getMangaIdFromTitle(std::string title){
  // url = https://api.mangadex.org"
  // params = {"title"= title}
  // response = curl get() or smth idk
  // return response.data("id") or smth idk
  return -1;
};

int mangadex_manga::downloadChapter(int chapter_index){
  return -1;//dummy func, for now...
};
  


