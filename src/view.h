typedef frame_buffer::rgb888 color888;
class image{
  private:
   color888 *addr;
   int bpp = 24;
   int size;
  public:
   image(int rows, int columns=480){
     width = columns;
     height = rows;
     size = width * height;
     addr = new color888[size];
   };
   int width;
   int height;
   ~image(){
     delete[] addr;
   };
   image(image&) = delete;
   constexpr color888& at(int row, int col){
     return addr[ row * width + col];
   };
   int display(int scroll);
};

int scale(double fctr, color888 *kernel, int w, int h, image *image);
