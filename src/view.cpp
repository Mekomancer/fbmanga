#include "view.h"

extern frame_buffer fb;

int image::display(int scroll){
  for(int col = 0; col < fb.vinfo.yres; col++){
    for(int row = 0; row < fb.vinfo.xres; row++){
      if(0<row+scroll&&row+scroll<height){
	fb.setPixel(fb.vinfo.yres-1-col, row, at(row+scroll,col));
      }
    }
  }
  return 0;
}

int scale(double fctr, color888 *kernel, int w, int h, image *img){
  double scl = 1/fctr - 0.1;
  for(int r = 0;  r < h; r++){
    for(int c = 0; c < h; c++){
      size_t i = static_cast<int>(c) + w * static_cast<int>(r);
      img->at(r*fctr,c*fctr) = kernel[i];
    }
  }
  return 0;
}
