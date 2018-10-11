#include "mandelbrot.hpp"

Mandelbrot::Mandelbrot(mpf_t x, mpf_t y, mpf_t w, mpf_t h, int im_w, int im_h, int supSample) : surEchantillonage(supSample), im_width(im_w), im_height(im_h)
{
	mpf_inits(this->pos_x, this->pos_y, this->width, this->height);
	mpf_set(this->pos_x, x);
	mpf_set(this->pos_y, y);
	mpf_set(this->width, w);
	mpf_set(this->height, h);
}

Mandelbrot::~Mandelbrot()
{
	mpf_clears(this->pos_x, this->pos_y, this->width, this->height);
}

void Mandelbrot::draw(int iterations)
{
	
}