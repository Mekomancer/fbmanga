const time_t access_token_lifetime = 60*15; // 15 mins, as of 09/03/25
const std::array<std::string,4> target_demographics = {"shounen","shoujo","josei","seinen"};
const std::array<std::string,4> publication_statuses = {"ongoing","completed","hiatus","cancelled"};
const std::array<std::string,6> reading_statuses = {"reading","on-hold","plan-to-read","dropped","re-reading","completed"};
const std::array<std::string,4> content_ratings = {"safe","suggestive","erotica","pornographic"};

class mangadex_auth {
 private:
  std::string username;
  std::string password;
  std::string client_id;
  std::string client_secret;
  std::string access_token;
  std::string refresh_token;
  time_t access_token_expiration_date;
  CURL *curl_handle;
 public:
  int setCreds(std::string username, std::string password, std::string id, std::string secret);
  int initTokens();//call setCreds before calling this func;
  std::string getAccessToken();//automatically refreshes token if expired;
  
  mangadex_auth();
};

class mangadex_manga {
 private:
   std::string id;
   std::string title;
 public:
   int getMangaIdFromTitle(std::string title);
   int downloadChapter(int chapter_index);
};
//returns manga id
std::string getMangaIdFromTags(std::vector<std::string> include_tags, std::vector<std::string> exclude_tags);

