#ifndef _MANDEL_
#define _MANDEL_

#include <iostream>
#include <iomanip>
	#include <stdlib.h>
	#include <stdio.h>

#include <gmp.h>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>

#include "color.hpp"
#include "load.hpp"

class Mandelbrot
{
	private:
		mpf_t pos_x, pos_y;
		mpf_t width,height;
		int im_width, im_height, iterations;
		int surEchantillonage;
		cv::Mat *divMat, *img;
	
	public:
		Mandelbrot(mpf_t x, mpf_t y, mpf_t w, mpf_t h, int im_w, int im_h, int supSample, int iterations);
		~Mandelbrot();
		void escapeSpeedCalc();
		void draw();
		void save();
		bool IsGood();
		void IterUp();
		//int DeepEnough(auto enough);
		void dichotomie(int enough);

};

#endif
