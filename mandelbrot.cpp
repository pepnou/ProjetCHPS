#include "mandelbrot.hpp"

using namespace cv;

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
	mpf_t atomic_w, atomic_h, xc, yc, xn, yn, xnp1, ynp1, mod, tmp;
	mpf_inits(atomic_w, atomic_h, xc, yc, xn, yn, xnp1, ynp1, mod, tmp);

	mpf_div_ui(atomic_w, this->width, this->im_width*this->surEchantillonage);
	mpf_div_ui(atomic_h, this->height, this->im_height*this->surEchantillonage);

	Mat* mat = new Mat(this->im_width*this->surEchantillonage, this->im_height*this->surEchantillonage, CV_8UC3);
	//Mat mat(this->im_width*this->surEchantillonage, this->im_height*this->surEchantillonage, CV_8UC3);

	for (int i = 0; i < this->im_width*this->surEchantillonage; ++i)
	{
		mpf_mul_ui(tmp, this->width, 2);
		mpf_sub(xc, this->pos_x, tmp);
		mpf_mul_ui(tmp, atomic_w, i);
		mpf_add(xc, xc, tmp);

		for (int j = 0; j < this->im_height*this->surEchantillonage; ++j)
		{
			mpf_mul_ui(tmp, this->height, 2);
			mpf_sub(yc, this->pos_y, tmp);
			mpf_mul_ui(tmp, atomic_h, j);
			mpf_add(yc, yc, tmp);

			for (int k = 1; k < iterations; ++k)
			{
				mpf_pow_ui(tmp, yn, 2);
				mpf_pow_ui(xnp1, xn, 2);
				mpf_sub(xnp1, xnp1, tmp);
				mpf_add(xnp1, xnp1, xc);

				mpf_mul(ynp1, xn, yn);
				mpf_mul_ui(ynp1, ynp1, 2);
				mpf_add(ynp1, ynp1, yn);

				mpf_pow_ui(mod, xnp1, 2);
				mpf_pow_ui(tmp, ynp1, 2);
				mpf_add(mod, mod, tmp);

				if(mpf_cmp_ui(mod, 4) > 0)
					break;
			}
			//Vec3b& rgb = mat->at<Vec3b>(i, j);
		}
	}









	mpf_clears(atomic_w, atomic_h, xc, yc, xn, yn, xnp1, ynp1, mod, tmp);
	delete mat;
	//delete &mat;
}