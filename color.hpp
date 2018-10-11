#ifndef _COLOR_
#define _COLOR_

#include <opencv2/core.hpp>
#include <cmath>

void RGBtoHSB();
cv::Vec3b HSBtoRGB( int h, int s, int v);
void coloration(cv::Vec3b &bgr,int n, int iterations);

#define dbg std::cout<<"line : "<<__LINE__<<", function : "<<__FUNCTION__<<"\n";

#endif