export module ui.gui;
import std;
import types;

export class image_t {
public:
  rgb888 &operator[](auto r, auto c);
  rgb888 &at(auto r, auto c);
  rgb888 *data() { return image_data.data(); };
  void resize(uint32_t w, uint32_t h);
  uint32_t xres() { return width; };
  uint32_t yres() { return height; };

private:
  uint32_t width;
  uint32_t height;
  std::vector<rgb888> image_data;
};

void image_t::resize(uint32_t w, uint32_t h) {
  width = w;
  height = h;
  image_data.resize(w * h);
}

rgb888 &image_t::operator[](auto row, auto col) {
  return image_data[row * width + col];
};
rgb888 &image_t::at(auto row, auto col) {
  if (row >= height) {
    throw std::out_of_range(
        std::format("row (which is {}) >= height (which is {})", row, height));

  } else if (row < 0) {
    throw std::out_of_range(std::format("row (which is {}) < 0", row));
  } else if (col >= width) {
    throw std::out_of_range(
        std::format("col (which is {}) >= width (which is {})", col, width));
  } else if (col < 0) {
    throw std::out_of_range(std::format("col (which is {}) < 0", col));
  } else {
    return operator[](row, col);
  }
}

void downscale_nearest(image_t *in, image_t *out) {
  for (uint32_t irow = 0; irow < in->yres(); ++irow) {
    uint32_t orow = irow * out->yres() / in->yres();
    for (uint32_t icol = 0; icol < in->xres(); ++icol) {
      uint32_t ocol = icol * out->xres() / in->xres();
      (*out)[orow, ocol] = (*in)[irow, icol];
    }
  }
}

//      cols
//   0 1 2 ... width
//  0xxxxxxxxxxxxxxxx
//  1xxxxxxxxxxxxxxxx
// r 2xxxxxxxxxxxxxxxx
// o .xxxxxxxxxxxxxxxx
// w .xxxxxxxxxxxxxxxx
// s .xxxxxxxxxxxxxxxx
//  hxxxxxxxxxxxxxxxx
//  exxxxxxxxxxxxxxxx
//  ixxxxxxxxxxxxxxxx
//  gxxxxxxxxxxxxxxxx
//  txxxxxxxxxxxxxxxx
//
//  >>>>>>
//  .----'
//  >>>>>>
//  .----'
//  >>>>>>
//
