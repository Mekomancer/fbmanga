#include "view.h"

extern frame_buffer fb;

int image::display(int scroll){
  for(int col = 0; col < fb.vinfo.yres; col++){
    for(int row = 0; row < fb.vinfo.xres; row++){
      fb.setPixel(fb.vinfo.yres-1-col, row, at(row+scroll,col));
    }
  }
  return 0;
}

int scale(double fctr, color888 *kernel, int w, int h, image *img){
  double scl = 1/fctr - 0.1;
  for(int r = 0;  r < img->height; r++){
    for(int c = 0; c < img->width; c++){
      long int i = static_cast<int>(c * scl) + w * static_cast<int>(r*scl);
      img->at(r,c) = kernel[i];
    }
  }
  return 0;
}

