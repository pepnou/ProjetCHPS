#ifndef _COLOR_
#define _COLOR_

#include <iostream>
#include <iomanip>

#include <opencv2/core.hpp>
#include <cmath>

void RGBtoHSB();
cv::Vec3b HSBtoRGB( int h, float s, float v);
void coloration(cv::Vec3b &bgr,int n, int iterations, int nbr_div, int nbr_ndiv);

#define dbg std::cout<<"line : "<<__LINE__<<", function : "<<__FUNCTION__<<"\n";

#endif