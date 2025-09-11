const std::array<uint8_t,8> png_file_signature = {137, 'P','N','G','\r','\n',26,'\n'};

struct png_chunk{
  uint32_t length;
  uint32_t type;
  char *data;
  uint32_t crc;
};

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

struct color565{
  uint8_t red:5; 
  uint8_t green:6; 
  uint8_t blue:5;
}__attribute__ ((packed));

static_assert(sizeof(color565)==2);

class png{
  private:
    char* addr;
    bool loaded = false;
    struct stat stats;
    std::vector<png_chunk> index;
    std::vector<color888> palette;
    uint32_t width;
    uint32_t height;
    uint8_t bit_depth;
    uint8_t color_type;
    uint8_t compression_method;
    uint8_t filter_method;
    uint8_t interlace_method;
  public:
    int load(std::string);//calls validate(), returns -1 if mmap() or validate() fails;
    int validate(int fd);
    int createIndex();
    int parseHeader();
    int loadPalette();
    png();//does nothing, use load();
    ~png();//munmap and other stuff;
};

