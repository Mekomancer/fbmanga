module;
#include <curl/curl.h>
export module net;
import types;
import debug;
export Result<void,int> initialize_curl(){
  CURLcode ret = curl_global_init(CURL_GLOBAL_ALL);
  if (ret != CURLE_OK) {
    log("error: curl_global_init returned {} ({})", here(),
        static_cast<int>(ret), curl_easy_strerror(ret));
    return Err(ret);
  }
  return{};
}
