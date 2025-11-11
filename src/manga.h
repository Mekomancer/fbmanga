class mangadex {
public:
  mangadex();
  bool checkup(); // returns true if the ping endpoint works
  int setCreds(std::string_view username, std::string_view password,
               std::string_view id, std::string_view secret);
  struct chapter_info {
    std::string id;
    std::string desc;
  };
  std::vector<chapter_info> getChapters(std::string_view manga_id);
  std::vector<std::string> getMangaId(std::string_view title = "acchi kocchi");
  std::vector<std::string> getImgUrls(std::string_view chapter);
  void downloadImg(std::string_view img_url, ring_buf *buf);
  int initTokens();
  std::string getAccessToken();
  static constexpr time_t access_token_lifetime = 60 * 15; // as of 09/03/25
private:
  void queryAdd(std::string_view param, std::string_view val);
  void clearQuery();
  void setEndpoint(std::string_view endpoint);
  void setEndpoint(std::string_view endpoint, std::string_view value);
  std::string getGroupName(std::string_view groupId);
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
