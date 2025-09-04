class framebuffer{
 private:
  uint16_t *addr;
  const int res = 320*480;
 public:
  uint16_t& operator[](int x, int y);
  int init();
};
