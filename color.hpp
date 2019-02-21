#ifndef _COLOR_
#define _COLOR_

#include <iostream>
#include <iomanip>

//#include <opencv2/core.hpp>
#include <cmath>

void RGBtoHSB();
//cv::Vec3b HSBtoRGB( int h, float s, float v);
void coloration(cv::Vec3b &bgr,int n, int iterations, int nbr_div, int nbr_ndiv);
void coloration2(cv::Vec3b &bgr,int n, int iterations);
void coloration3(cv::Vec3b &bgr,int n, int iterations);

#define dbg std::cout<<"line : "<<__LINE__<<", function : "<<__FUNCTION__<<"\n";

#endif
