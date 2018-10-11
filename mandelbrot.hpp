#ifndef _MANDEL_
#define _MANDEL_

#include <iostream>
#include <complex>

#include <gmp.h>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

class Mandelbrot
{
	private:
		std::complex<long double> pos;
		//Mat M;
		double width;
		int im_width;
		int surEchantillonage;
	
	public:
		void move();
			//random
			//random walk assisté en suiavtn les bords
		void draw(int iterations);
		bool isNice();
};

#endif
