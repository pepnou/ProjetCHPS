#include "mandelbrot.hpp"

using namespace cv;
using namespace std;

Mandelbrot::Mandelbrot(mpf_t x, mpf_t y, mpf_t w, mpf_t h, int im_w, int im_h, int supSample, int iterations) : surEchantillonage(supSample), im_width(im_w), im_height(im_h), iterations(iterations)
{
	mpf_inits(this->pos_x, this->pos_y, this->width, this->height, NULL);
	
	mpf_set(this->pos_x, x);
	mpf_set(this->pos_y, y);
	mpf_set(this->width, w);
	mpf_set(this->height, h);

	this->divMat = new Mat(im_h*supSample, im_w*supSample, CV_32FC1);
	this->img = new Mat(im_h, im_w, CV_8UC3);
}

Mandelbrot::~Mandelbrot()
{
	mpf_clears(this->pos_x, this->pos_y, this->width, this->height);
	delete this->divMat;
	delete this->img;
}

void Mandelbrot::escapeSpeedCalc()
{
	mpf_t atomic_w, atomic_h, xc, yc, xn, yn, xnp1, ynp1, mod, tmp;
	mpf_inits(atomic_w, atomic_h, xc, yc, xn, yn, xnp1, ynp1, mod, tmp, NULL);

	//  atomic_w = width / (im_width * surEchantillonage)
	//  atomic_h = height / (im_height * surEchantillonage)
	mpf_div_ui(atomic_w, this->width, this->im_width*this->surEchantillonage);
	mpf_div_ui(atomic_h, this->height, this->im_height*this->surEchantillonage);

    loading(this->im_width * this->surEchantillonage * this->im_height * this->surEchantillonage);
    loading(0);

	for (int i = 0; i < this->im_width*this->surEchantillonage; ++i)
	{
		//  xc = pos_x - width/2 + i*atomic_w
		mpf_div_ui(tmp, this->width, 2); //  tmp = width/2
		mpf_sub(xc, this->pos_x, tmp); //  xc = pos_x - tmp = pos_x - width/2
		mpf_mul_ui(tmp, atomic_w, i); //  tmp = atomic_w * 1
		mpf_add(xc, xc, tmp); //  xc = xc + tmp = pos_x - width/2 + atomic_w * i

		for (int j = 0; j < this->im_height*this->surEchantillonage; ++j)
		{
			//  yc = pos_y - height/2 + i*atomic_h
			mpf_div_ui(tmp, this->height, 2); //  tmp = height/2
			mpf_sub(yc, this->pos_y, tmp); //  yc = pos_y - tmp = pos_y - height/2
			mpf_mul_ui(tmp, atomic_h, j); //  tmp = atomic_h * j
			mpf_add(yc, yc, tmp); //  yc = yc + tmp = pos_y - height/2 + atomic_h * j

			mpf_set_ui(xn,0);
			mpf_set_ui(yn,0);

			for (int k = 1; k < this->iterations; ++k)
			{
				//  xnp1 = xn² - yn² + xc
				mpf_pow_ui(tmp, yn, 2); //  tmp = yn²
				mpf_pow_ui(xnp1, xn, 2); //  xnp1 = xn²
				mpf_sub(xnp1, xnp1, tmp); //  xnp1 = xnp1 - tmp = xn² - yn²
				mpf_add(xnp1, xnp1, xc); //  xnp1 = xnp1 + xc = xn² - yn² + xc

				//  ynp1 = 2*xn*yn + yc
				mpf_mul(ynp1, xn, yn); //  ynp1 = xn * yn
				mpf_mul_ui(ynp1, ynp1, 2); //  ynp1 = ynp1 * 2 = 2 * xn * yn
				mpf_add(ynp1, ynp1, yc); //  ynp1 = ynp1 + yc = 2 * xn * yn + yc

				//  mod = xnp1² + ynp1²
				mpf_pow_ui(mod, xnp1, 2); //  mod = xnp1²
				mpf_pow_ui(tmp, ynp1, 2); //  tmp = ynp1²
				mpf_add(mod, mod, tmp); //  mod = mod + tmp = xnp1² + ynp1²

				//  xn = xnp1
				//  yn = ynp1
				mpf_set( xn, xnp1); //  xn = xnp1
				mpf_set( yn, ynp1); //  yn = ynp1

				if(mpf_cmp_ui(mod, 4) > 0)
				{
					this->divMat->at<int>(j, i) = k;
					break;
				} else if(k == this->iterations -1)
					{
						this->divMat->at<int>(j, i) = this->iterations;
					}
			}
			loading(i*(this->im_height*this->surEchantillonage) + j);
		}
	}
	
	mpf_clears(atomic_w, atomic_h, xc, yc, xn, yn, xnp1, ynp1, mod, tmp, NULL);
}

void Mandelbrot::draw()
{
	int moy, nbr_div, nbr_ndiv, divSpeed;

	for(int i = 0; i < this->im_width; ++i)
	{
		for (int j = 0; j < this->im_height; ++j)
		{
			moy = 0, nbr_div = 0, nbr_ndiv = 0;
			for(int k = 0; k < this->surEchantillonage; k++)
			{
				for(int l = 0; l < this->surEchantillonage; l++)
				{
					divSpeed = divMat->at<int>( j*this->surEchantillonage + l, i*this->surEchantillonage + k);
					if(divSpeed == this->iterations)
						nbr_ndiv++;
					else
					{
						moy += divSpeed;
						nbr_div++;
					}
				}
			}
			if(nbr_div)
				moy /= nbr_div;

			Vec3b& bgr = this->img->at<Vec3b>( j, i);
			coloration(bgr, moy, this->iterations, nbr_div, nbr_ndiv);
		}
	}
}

void Mandelbrot::save()
{
	vector<int> compression_params;
    compression_params.push_back( IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(9);

	try
    {
        imwrite("mandel.png", *(this->img), compression_params);
    }
    catch (const cv::Exception& ex)
    {
        fprintf(stderr, "Exception converting image to PNG format: %s\n", ex.what());
    }
}