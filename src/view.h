#include "util.h"
#include "fb.h"

class image{
  private:
   std::vector<rgb888> data;
   int bpp = 24;
   int size;
  public:
   void resize(size_t count){
     data.resize(count);
   }
   int width;
   int height;
   constexpr rgb888& at(int row, int col){
     return data[ row * width + col];
   };
   int scale(double fctr, std::span<rgb888> kernel, int w, int h);
   int display(int scroll);
};

