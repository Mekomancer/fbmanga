class mangadex {
public:
  mangadex();
  bool checkup(); // returns true if the ping endpoint works
  std::vector<std::string> getScanlationGroups(std::string chap_id);
  int setCreds(std::string_view username, std::string_view password,
               std::string_view id, std::string_view secret);
  std::vector<std::string> getChapterIds(std::string_view manga_id);
  std::vector<std::string> getMangaId(std::string_view title = "acchi kocchi");
  /*fight me, i dare u. i own this repo, i make the decsions.*/
  std::vector<int> downloadChapter(std::string_view manga);
  std::vector<std::string> getImgUrls(std::string_view chapter);
  int initTokens();             // call setCreds before calling this func;
  std::string getAccessToken(); // automatically refreshes token if expired;
  const time_t access_token_lifetime = 60 * 15; // 15 mins, as of 09/03/25

private:
  void queryAdd(std::string_view param, std::string_view val);
  void clearQuery();
  void setEndpoint(std::string_view endpoint);
  void setEndpoint(std::string_view endpoint, std::string_view value);
  CURLU *url;
  std::string username;
  std::string password;
  std::string client_id;
  std::string client_secret;
  std::string access_token;
  std::string refresh_token;
  time_t access_token_expiration_date;
  CURL *curl;
  void prepareCurl();
};
