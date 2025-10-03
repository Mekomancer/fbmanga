#include "stream.h"

typedef uint16_t pixel_t;

const char png_signature[] = {137, 'P','N','G','\r','\n',26,'\n'};

extern std::map<std::string, std::array<char,4>> chunk_type;

enum class color_type_t: uint8_t{
  greyscale 		= 0,
  greyscale_indexed	= 1, 
  truecolor 		= 2,
  palette  		= 3,
  greyscale_alpha	= 4,
  truecolor_alpha	= 6,
};

std::string to_string(color_type_t val); 

class png_t{
  private:
    datastream *data;
    struct stats_t {
      uint32_t width;
      uint32_t height;
      uint8_t bit_depth;
      color_type_t color_type;
      uint8_t compression_method;
      uint8_t filter_method;
      uint8_t interlace_method;
    } stats;
    std::vector<uint16_t> palette;
    int decodeHeader();
    int decodePalette();
    int decompress();
  public:
    int decode(datastream *png_data);
};

