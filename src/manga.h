class mangadex{
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
  const std::string base_url = "https://api.mangadex.org";
  const time_t access_token_lifetime = 60*15; // 15 mins, as of 09/03/25
  const std::array<std::string,4> target_demographics = {"shounen","shoujo","josei","seinen"};
  const std::array<std::string,4> publication_statuses = {"ongoing","completed","hiatus","cancelled"};
  const std::array<std::string,6> reading_statuses = {"reading","on-hold","plan-to-read","dropped","re-reading","completed"};
  const std::array<std::string,4> content_ratings = {"safe","suggestive","erotica","pornographic"};
  int setCreds(std::string username, std::string password, std::string id, std::string secret);
  std::vector<std::string> getChapterIds(std::string manga_id);
  std::vector<std::string> getMangaId(std::string title="acchi kocchi");
  /*fight me, i dare u. i own this repo, i make the decsions.*/
  std::vector<int> downloadChapter(std::string manga);
  std::vector<std::string> getImgUrls(std::string chapter);
  int initTokens();//call setCreds before calling this func;
  std::string getAccessToken();//automatically refreshes token if expired;
  int init();
};


