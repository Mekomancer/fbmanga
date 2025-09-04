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


  


