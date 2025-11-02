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

int image::scale(double fctr, std::span<rgb888> kernel, int w, int h){
  double scl = 1/fctr - 0.1;
  for(int r = 0;  r < height; r++){
    for(int c = 0; c < width; c++){
      long int i = static_cast<int>(c * scl) + w * static_cast<int>(r*scl);
      at(r,c) = kernel[i];
    }	
  }
  return 0;
}

