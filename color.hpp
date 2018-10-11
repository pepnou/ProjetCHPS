#ifndef _COLOR_
#define _COLOR_

#include <opencv2/core.hpp>

void RGBtoHSB();
void HSBtoRGB();
void coloration(cv::Vec3b bgr,int n, int iterations);

#endif