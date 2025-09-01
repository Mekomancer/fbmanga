class framebuffer{
  private:
    short* addr;
  public:
    short& operator[](int x, int y);
    int init(std::string path);
};
