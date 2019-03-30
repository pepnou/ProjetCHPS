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
#include <mpfr.h>
#include <mpf2mpfr.h>
#include <gmp.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <map>


#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/imgcodecs.hpp>

#include "color.hpp"
#include "rdtsc.hpp"
#include "matOp.hpp"

typedef struct
{
    double key;
    char* val;
}keyed_char;

class Mandelbrot
{
	private:
		mpfr_t pos_x, pos_y;
		mpfr_t width,height;
		mpfr_t atomic_w, atomic_h;
		int iterations, enough;
                double ThresholdCont, ThresholdSave;
                std::vector<int> divs;
		
                
                cv::Mat *divMat, *img, *sEMat;
                
		void calcSeq(mpfr_t* x, mpfr_t* y);
		void calcPar(mpfr_t* x, mpfr_t* y);
	public:
		Mandelbrot(mpfr_t x, mpfr_t y, mpfr_t w, mpfr_t h, int _enough, std::vector<int> divs);
                Mandelbrot(char* buf);
		~Mandelbrot();
		void del_mem();
    
                void escapeSpeedCalcSeq();
		void escapeSpeedCalcPar();

		void draw();
		void save(int img_num, int start);
		bool IsGood();
		void IterUp();
		bool IsGood_2(bool* filtre, double* res2);
		
                void dichotomie3();
		bool random_img (int enough, double zoom, gmp_randstate_t& state);

		// static int pas;
                static int surEchantillonage;
                static int im_width, im_height;
		static int color;
		static char* rep;

                //static std::multimap<double, char*> map;
                static keyed_char* top10;
};

#endif