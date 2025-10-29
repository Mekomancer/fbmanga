struct page_t{
  std::shared_ptr<std::byte> addr;
  page_t();	
};


template<typename data>
class ring_buf{
  private:
    std::vector<page_t> chunks;
  public:
};



typedef frame_buffer::rgb888 color888;
class image{
  private:
   std::vector<color888> data;
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
   constexpr color888& at(int row, int col){
     return data[ row * width + col];
   };
   int scale(double fctr, std::span<color888> kernel, int w, int h);
   int display(int scroll);
};

