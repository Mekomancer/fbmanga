const char png_file_signature[] = {137, 'P','N','G','\r','\n',26,'\n'};
enum class chunk_type: uint32_t{
  IHDR = 0x52444849,//0x49 48 44 52,
  IDAT = 0x54414449,//0x49 44 41 54,
  PLTE = 0x45544C50,//0x50 4C 54 45,
};
struct png_chunk{
  uint32_t length;
  chunk_type type;
  char *data;
  uint32_t crc;
};

enum class png_pixel_type{
  greyscale = 0,
  truecolor =2,
  palette =3,
  greyscale_alpha=4,
  truecolor_alpha=6,
};

std::string to_string(png_pixel_type val); 

class png{
  private:
    bool loaded;
    char* png_addr;
    char* img_addr;
    size_t img_mem_len;
    size_t png_size;
    struct stat stats;
    std::vector<png_chunk> index;
    std::vector<uint16_t> palette;
    int palette_index;
    uint32_t width;
    uint32_t height;
    uint8_t bit_depth;
    png_pixel_type color_type;
    uint8_t compression_method;
    uint8_t filter_method;
    uint8_t interlace_method;
    int validate();
    int checkOrdering();
    int createIndex();
    int parseHeader();
    int loadPalette(png_chunk chunk);
    int getData();
  public:
    int load(std::string);// returns -1 if it fails;
    img extractImage();
    png();//does nothing, use load();
    ~png();//munmap and other stuff;
};

