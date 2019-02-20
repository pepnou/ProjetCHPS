#ifndef _MANDEL_
#define _MANDEL_

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstdio>
#include <thread>
#include <vector>
#include <ctime>
#include <fstream>
#include <gmp.h>
#include <string>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>

#include "color.hpp"
#include "rdtsc.hpp"
#include "matOp.hpp"


class Mandelbrot
{
	private:
		mpf_t pos_x, pos_y;
		mpf_t width,height;
		mpf_t atomic_w, atomic_h;
		int iterations, enough;
                double ThresholdCont, ThresholdSave;

		
                
                cv::Mat *divMat, *img, *sEMat;
                
		void calcSeq(mpf_t* x, mpf_t* y);
	public:
		Mandelbrot(mpf_t x, mpf_t y, mpf_t w, mpf_t h, int _enough);
                Mandelbrot(char* buf);
		~Mandelbrot();
		void del_mem();
    
                void escapeSpeedCalcSeq();

		void draw();
		void save();
		bool IsGood();
		void IterUp();
		bool IsGood_2(bool* filtre);
		
                void dichotomie3(int n_div, std::vector<int>& divs, int prec);
		bool random_img (int enough, double zoom, gmp_randstate_t& state);

		static int pas;
                static int surEchantillonage;
                static int im_width, im_height;
		static int color;
		static char* rep;
};

#endif
