#include "util.h"
#include "fb.h"

class image{
  private:
   std::vector<rgb888> data;
   int bpp = 24;
   int size;
  public:
   image(int rows, int columns=480){
     width = columns;
     height = rows;
     size = width * height;
     data.resize(size);
   };
   int width;
   int height;
   image(image&) = delete;
   constexpr rgb888& at(int row, int col){
     return data[ row * width + col];
   };
   int scale(double fctr, std::span<rgb888> kernel, int w, int h);
   int display(int scroll);
};

