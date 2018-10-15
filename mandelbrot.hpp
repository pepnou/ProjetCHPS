#ifndef _MANDEL_
#define _MANDEL_

#include <iostream>
#include <iomanip>
#include <thread>
#include <vector>

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
		mpf_t atomic_w, atomic_h;
		int im_width, im_height, iterations;
		int surEchantillonage;
		cv::Mat *divMat, *img;
		
		//void* threadCalc(void* arg);
		void threadCalc(int j);
		
	public:
		Mandelbrot(mpf_t x, mpf_t y, mpf_t w, mpf_t h, int im_w, int im_h, int supSample, int iterations);
		~Mandelbrot();
		void escapeSpeedCalc();
		void escapeSpeedCalcThread();
		void draw();
		void save();
		bool isNice();
};

#endif
