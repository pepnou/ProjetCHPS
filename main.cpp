#include "main.hpp"

using namespace cv;
using namespace std;




int main(int argc, char** argv)
{
	mpf_t x, y, w, h;
	mpf_init_set_d(x,0.0);
	mpf_init_set_d(y,0.0);
	mpf_init_set_d(w,4.0);
	mpf_init_set_d(h,4.0);

	Mandelbrot M(x,y,w,h,1920,1080,1);
	M.draw(100);

	mpf_clears( x, y, w, h, NULL);
	return 0;
}