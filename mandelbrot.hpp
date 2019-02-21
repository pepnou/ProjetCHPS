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

/*
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
*/

#include "color.hpp"
#include "rdtsc.hpp"
#include "matOp.hpp"
#include "mpmc.hpp"

class Mpmc;

class Mandelbrot
{
	private:
		mpf_t pos_x, pos_y;
		mpf_t width,height;
		mpf_t atomic_w, atomic_h;
		int im_width, im_height, iterations;
		int surEchantillonage;
		double ThresholdCont, ThresholdSave;
		int color;
		cv::Mat *divMat;
		cv::Mat *img;
		cv::Mat *sEMat;		
		char* rep;

		Mpmc* mpmc;
		std::atomic<int> tasks;

		void threadCalc(int deb, int fin);
		void threadCalc2(int deb, int fin, mpf_t* x, mpf_t* y);
		void threadCalc2_2(int deb, int fin, mpf_t* x, mpf_t* y);
		void threadCalc3(int deb, int fin, mpf_t* x, mpf_t* y);
		void threadCalc4(void* arg);

		static void CallThreadCalc(void* arg);
		
	public:
		Mandelbrot(mpf_t x, mpf_t y, mpf_t w, mpf_t h, int im_w, int im_h, int supSample, int iterations, int color, Mpmc* mpmc,  char* rep = nullptr);
		~Mandelbrot();
		void del_mem();
		void escapeSpeedCalc();
		void escapeSpeedCalcThread();
		void escapeSpeedCalcThread2();
		void escapeSpeedCalcThread3();
		void escapeSpeedCalcThread4();
		void draw();
		void save();
		bool IsGood();
		void IterUp();
		bool IsGood_2(bool* filtre);
		
		void dichotomie(int enough, int prec);
		void dichotomie2(int enough, int n_div, std::vector<int>& divs, int prec);
		bool random_img (int enough, double zoom, gmp_randstate_t& state);

		static int pas;
};

#endif
