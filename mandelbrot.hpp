#ifndef _MANDEL_
#define _MANDEL_

#include <iostream>
#include <complex>

#include <gmp.h>

using namespace std;

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>

class Mandelbrot
{
	private:
		mpf_t pos_x, pos_y;
		mpf_t width,height;
		int im_width,im_height;
		int surEchantillonage;
	
	public:
		void draw(int iterations);
		bool isNice();
};

static void createAlphaMat(cv::Mat &mat)
{
    CV_Assert(mat.channels() == 4);
    for (int i = 0; i < mat.rows; ++i)
    {
        for (int j = 0; j < mat.cols; ++j)
        {
            cv::Vec4b& bgra = mat.at<cv::Vec4b>(i, j);
            bgra[0] = UCHAR_MAX; // Blue
            bgra[1] = cv::saturate_cast<uchar>((float (mat.cols - j)) / ((float)mat.cols) * UCHAR_MAX); // Green
            bgra[2] = cv::saturate_cast<uchar>((float (mat.rows - i)) / ((float)mat.rows) * UCHAR_MAX); // Red
            bgra[3] = cv::saturate_cast<uchar>(0.5 * (bgra[1] + bgra[2])); // Alpha
        }
    }
}

void test()
{
	// Create mat with alpha channel
    cv::Mat mat(480, 640, CV_8UC4);
    createAlphaMat(mat);
    vector<int> compression_params;
    compression_params.push_back(cv::IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(9);
    bool result = false;
    try
    {
        result = imwrite("alpha.png", mat, compression_params);
    }
    catch (const cv::Exception& ex)
    {
        fprintf(stderr, "Exception converting image to PNG format: %s\n", ex.what());
    }
    if (result)
        printf("Saved PNG file with alpha data.\n");
    else
        printf("ERROR: Can't save PNG file.\n");
}

#endif
