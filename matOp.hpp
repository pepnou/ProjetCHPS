#ifndef _MATOP_
#define _MATOP_

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstdio>
#include <thread>
#include <vector>

#include <gmp.h>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

int matSave(cv::Mat* mat, char* rep);

#endif