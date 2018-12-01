#ifndef _MANDEL_
#define _MANDEL_

#define ITERATIONS_PER_THREAD 400000000

// 47x28(1 316) =>  (100it)
// 120x67(8 040) =>  (100it)
// 240x135(32 400) =>  (100it)
// 480x270(129 600) =>  (100it)
// 1920x1080(2 073 600) =>  (100it)

// 47x28(1 316) => 10 (50it)
// 120x67(8 040) => 14 (50it)
// 240x135(32 400) => 9 (50it)
// 480x270(129 600) => 5 (50it)
// 1920x1080(2 073 600) => 3 (50it)

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
#include "load.hpp"
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
		cv::Mat *divMat, *img, *sEMat;
		char* rep;

		Mpmc* mpmc;
		std::atomic<int> tasks;

		// void* threadCalc(void* arg);
		void threadCalc(int deb, int fin);
		void threadCalc2(int deb, int fin, mpf_t* x, mpf_t* y);
		void threadCalc2_2(int deb, int fin, mpf_t* x, mpf_t* y);
		void threadCalc3(int deb, int fin, mpf_t* x, mpf_t* y);
		void threadCalc4(void* arg);
		// void partialDraw();
		friend void CallThreadCalc(void* arg);
		
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
		// void draw2();
		void save(double t);
		bool IsGood();
		void IterUp();
		bool IsGood_2(bool* filtre, double* t1, double* t2);
		//int DeepEnough(auto enough);
		void dichotomie(int enough, int prec);
		bool alea(int enough, int prec);
		void video();
};

void CallThreadCalc(void* arg);

#endif
