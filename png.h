const std::array<char,8> png_sig{137, 'P','N','G','\r','\n',26,'\n'};

extern std::map<std::string, std::array<char,4>> chunk_type;

enum class color_type:uint8_t{
  greyscale 		= 0,
  truecolor 		= 2,
  palette  		= 3,
  greyscale_alpha	= 4,
  truecolor_alpha	= 6,
};

constexpr std::string colorTypeString(color_type val); 

enum class filter_type:uint8_t{
  none = 0,
  sub,
  up,
  average,
  paeth,
};

    
class png{
  private:
    struct stats_t {
      uint32_t width;
      uint32_t height;
      uint8_t bit_depth;
      color_type color_type;
      uint8_t compression_method;
      uint8_t filter_method;
      uint8_t interlace_method;
    } stats;
    std::vector<uint16_t> palette;
    int decodeHeader();
    int decodePalette(uint32_t length);
    int decodeImageData(uint32_t length);
    std::function<int(char*,int)> _read;
    std::vector<uint8_t> curline;
    std::vector<uint8_t> prevline;
    int scanline_mem;
    int bpp;
    int filterline(uint8_t *buf, int length);
    int read(auto *buf){return _read(reinterpret_cast<char*>(buf),sizeof(*buf));};
    int read(auto *buf,auto length){return _read(reinterpret_cast<char*>(buf),length);};
  public:
    int decode(std::function<int(char*,int)> readfunc);
};


