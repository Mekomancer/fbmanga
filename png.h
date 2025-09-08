const std::array<uint8_t,8> png_file_signature = {137, 'P','N','G','\r','\n',26,'\n'};
enum class png_pixel_type{
  grayscale = 0,
  palette =3,
  truecolor =2,
  alpha,//yes this ',' is legal
};

struct color888{
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};

struct IHDR{
  uint32_t length;
  char type[4];
  uint32_t width;
  uint32_t height;
  uint8_t bit_depth;
  uint8_t color_type;
  uint8_t compression_method;
  uint8_t filter_method;
  uint8_t interlace_method;
  uint32_t CRC;
};

class png{
  private:
    char* addr;
    char* cur;
    struct stat stats;
    IHDR header;
    std::vector<color888> palette;
  public:
    int load(std::string);//calls validate(), returns -1 if mmap() or validate() fails;
    int validate(int fd);
    char& operator[](int i);
    int parseIHDR();
    int skimUntilIDAT();
    int chunkDataLength();
    int loadPalette();
    png();//does nothing use load();
    ~png();//munmap and other stuff;
};
