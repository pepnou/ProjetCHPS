#ifndef _MANDEL_
#define _MANDEL_

#include <iostream>
#include <complex>

//#include <gmp> ???
//#include <opencv>

class Mandelbrot
{
	private:
		std::complex<long double> pos;
		//Mat M;
		long double width;
		int im_width;
		int sur-echantillonage;
	
	public:
		void move();
			//random
			//random walk assisté en suiavtn les bords
		void draw(int iterations);
		bool isNice();
}

#endif
