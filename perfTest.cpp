#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <stdio.h>
#include <thread>
#include <vector>

#include <gmp.h>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>

#include "rdtsc.hpp"

using namespace std;
using namespace cv;

int main(int argc, char** argv)
{
	int nbr_test=10000;
	uint64_t tick, total;
	
	/*
	int w=1000, h=1000;
	Mat* mat;
	int** tab;
	int* tab2;

	tick = rdtsc();
	for(int i; i< nbr_test; i++)
	{
		mat = new Mat( h, w, CV_32FC1);
		delete mat;
	}
	cout<<"Cout allocation/deallocation cv::MAt"<<(rdtsc() - tick)/nbr_test<<endl;
	
	tick = rdtsc();
	for(int i; i< nbr_test; i++)
	{
		tab = new int*[w];
		for(int j = 0; j < w; j++)
		{
			tab[j] = new int[h];
		}
		
		for(int j = 0; j < j; j++)
		{
			delete [] tab[j];
		}
		delete [] tab;
	}
	cout<<"Cout allocation/deallocation int**"<<(rdtsc() - tick)/nbr_test<<endl;
	
	tick = rdtsc();
	for(int i; i< nbr_test; i++)
	{
		tab2 = new int[w*h];
		
		delete [] tab2;
	}
	cout<<"Cout allocation/deallocation int*"<<(rdtsc() - tick)/nbr_test<<endl;*/
	
	
	/*gmp_randstate_t state;
	gmp_randinit_default (state);
	int prec = 2048;

	mpf_t a,b,c;
	mpf_init2( a, 2048);
	mpf_init2( b, 2048);
	mpf_init2( c, 2048);

	total = 0;
	for(int i=0; i<nbr_test;i++)
	{
		mpf_urandomb( a, state, prec);
		mpf_urandomb( b, state, prec);
		
		tick = rdtsc();

		total += rdtsc() - tick(); 
	}
	cout << ""

	

	mpf_add(mpf t rop, const mpf t op1, const mpf t op2) 
	mpf_add_ui (mpf t rop, const mpf t op1, unsigned long int op2) 
	mpf_sub (mpf t rop, const mpf t op1, const mpf t op2) 
	mpf_ui_sub (mpf t rop, unsigned long int op1, const mpf t op2) 
	mpf_sub_ui (mpf t rop, const mpf t op1, unsigned long int op2) 
	mpf_mul (mpf t rop, const mpf t op1, const mpf t op2) 
	mpf_mul_ui (mpf t rop, const mpf t op1, unsigned long int op2) 
	mpf_div (mpf t rop, const mpf t op1, const mpf t op2) 
	mpf_ui_div (mpf t rop, unsigned long int op1, const mpf t op2) 
	mpf_div_ui (mpf t rop, const mpf t op1, unsigned long int op2) 
	mpf_sqrt (mpf t rop, const mpf t op) 
	mpf_sqrt_ui (mpf t rop, unsigned long int op) 
	mpf_pow_ui (mpf t rop, const mpf t op1, unsigned long int op2) 
	mpf_neg (mpf t rop, const mpf t op) 
	mpf_abs (mpf t rop, const mpf t op) 
	mpf_mul_2exp (mpf t rop, const mpf t op1, mp bitcnt t op2) 
	mpf_div_2exp (mpf t rop, const mpf t op1, mp bitcnt t op2)*/


	exit(0);
}
