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
	/*int w=1000, h=1000, nbr_test=10000;
	Mat* mat;
	int** tab;
	int* tab2;
	uint64_t tick;
	
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
	
	
	
	
	exit(0);
}
