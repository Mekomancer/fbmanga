
class frame_buffer {
  private:
    void *addr;
    int fd;
  public:
    struct rgb888{
      uint8_t red;
      uint8_t green;
      uint8_t blue;
    };
    bool constexpr inBounds(int row, int col){
      return (0<=row&&row<vinfo.yres&&0<=col&&col<vinfo.xres);
    }
    fb_var_screeninfo vinfo;
    fb_fix_screeninfo finfo;
    void printInfo();
    int init(std::string fb_device = "/dev/fb0");
    int free();
    void setPixel(int row, int col, rgb888 color) noexcept;
    rgb888 getPixel(int row, int col);
};
