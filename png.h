template <typename type>
constexpr type ntoh(type val){
  return std::byteswap<type>(val);
}

const char png_signature[] = {137, 'P','N','G','\r','\n',26,'\n'};

extern std::map<std::string, std::array<char,4>> chunk_type;

enum class color_type{
  greyscale 		= 0,
  truecolor 		= 2,
  palette  		= 3,
  greyscale_alpha	= 4,
  truecolor_alpha	= 6,
};

std::string to_string(color_type val); 

class png{
  private:
    std::istream *datastream;
    struct stats {
      uint32_t width;
      uint32_t height;
      uint8_t bit_depth;
      uint8_t color_type;
      uint8_t compression_method;
      uint8_t filter_method;
      uint8_t interlace_method;
    };
    std::vector<uint16_t> palette;
    int decodeHeader();
    int decodePalette();
    int decompress();
    template <typename field_type>
    field_type readIn(int num_bytes);
  public:
    int decode(std::ostream img_datastream);
    png(std::istream png_datastream);
    ~png();
};

