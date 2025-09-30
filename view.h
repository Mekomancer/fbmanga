class image{
  private:
    int size;
};


class framebuffer{
 private:
  uint16_t *addr;
  const int res = 320*480;
 public:
  uint16_t& operator[](int x, int y);
  int init();
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

constexpr uint16_t rgb888topixel(uint8_t red, uint8_t green, uint8_t blue){
  return  (static_cast<uint16_t>(red>>3) << 11 ) +
    (static_cast<uint16_t>(green>>2) << 5)  +
    static_cast<uint16_t>( blue >> 3 );

};
// this took much longer to get correct than i'd like to admit ToT
//                 b b b b b b b b 1 2 3  
//                       b b b b b b b b        
// f e d c b a 9 8 7 6 5 4 3 2 1 0
//                       b b b b b
//
//                 g g g g g g g g   
//                     g g g g g g 
//           g g g g g g 5 4 3 2 1
// f e d c b a 9 8 7 6 5 4 3 2 1 0
//           g g g g g g          
//
//                 r r r r r r r r    
//                       r r r r r 
// f e d c b a 9 8 7 6 5 4 3 2 1 0
// r r r r r b a 9 8 7 6 5 4 3 2 1
