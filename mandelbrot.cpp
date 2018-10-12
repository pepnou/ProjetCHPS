#include "mandelbrot.hpp"

using namespace cv;
using namespace std;

Mandelbrot::Mandelbrot(mpf_t x, mpf_t y, mpf_t w, mpf_t h, int im_w, int im_h, int supSample) : surEchantillonage(supSample), im_width(im_w), im_height(im_h)
{
	mpf_inits(this->pos_x, this->pos_y, this->width, this->height, NULL);
	
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
	mpf_inits(atomic_w, atomic_h, xc, yc, xn, yn, xnp1, ynp1, mod, tmp, NULL);

	
	//  atomic_w = width / (im_width * surEchantillonage)
	//  atomic_h = height / (im_height * surEchantillonage)
	mpf_div_ui(atomic_w, this->width, this->im_width*this->surEchantillonage);
	mpf_div_ui(atomic_h, this->height, this->im_height*this->surEchantillonage);

	Mat* mat = new Mat( this->im_height*this->surEchantillonage, this->im_width*this->surEchantillonage, CV_8UC3);
	//Mat mat(this->im_width*this->surEchantillonage, this->im_height*this->surEchantillonage, CV_8UC3);

	Mat* img = new Mat( this->im_height, this->im_width, CV_8UC3);
	//Mat img(this->im_width, this->im_height, CV_8UC3);

	vector<int> compression_params;
    compression_params.push_back( IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(9);

    /*cout<<mpf_get_d(this->width)<<" "<<mpf_get_d(this->height)<<endl;
    cout<<mpf_get_d(atomic_w)<<" "<<mpf_get_d(atomic_h)<<endl;
    cout<<mat->rows<<" "<<mat->cols<<endl;*/

    Vec3b tmpvec;

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

			for (int k = 1; k < iterations; ++k)
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
					coloration(mat->at<Vec3b>(j, i), k, iterations);
					break;
				} else if(k == iterations -1)
					{
						coloration(mat->at<Vec3b>(j, i), iterations, iterations);
					}
			}
			loading(i*(this->im_height*this->surEchantillonage) + j);
		}
		//cout<<endl;
	}

	try
    {
        imwrite("mandelprev.png", *mat, compression_params);
    }
    catch (const cv::Exception& ex)
    {
        fprintf(stderr, "Exception converting image to PNG format: %s\n", ex.what());
    }
	
	int moy_r, moy_g, moy_b;

	for(int i = 0; i < this->im_width; ++i)
	{
		for (int j = 0; j < this->im_height; ++j)
		{
			moy_r = 0; moy_g = 0; moy_b = 0;
			for(int k = 0; k < this->surEchantillonage; k++)
			{
				for(int l = 0; l < this->surEchantillonage; l++)
				{
					tmpvec = mat->at<Vec3b>( j*this->surEchantillonage+l, i*this->surEchantillonage+k);
					moy_b += tmpvec[0];
					moy_g += tmpvec[1];
					moy_r += tmpvec[2];
				}
			}
			Vec3b& bgr = img->at<Vec3b>( j, i);

			bgr[0] = moy_b/(this->surEchantillonage * this->surEchantillonage);
			bgr[1] = moy_g/(this->surEchantillonage * this->surEchantillonage);
			bgr[2] = moy_r/(this->surEchantillonage * this->surEchantillonage);
		}
	}


	/*char test_window[] = "test Mandelbrot";
	imshow( test_window, *img);
	waitKey( 0);*/


    try
    {
        imwrite("mandel.png", *img, compression_params);
    }
    catch (const cv::Exception& ex)
    {
        fprintf(stderr, "Exception converting image to PNG format: %s\n", ex.what());
    }

	//mpf_clears(atomic_w, atomic_h, xc, yc, xn, yn, xnp1, ynp1, mod, tmp, NULL);
	
	delete mat;
	//delete &mat;
	
	delete img;
	//delete &img;
}