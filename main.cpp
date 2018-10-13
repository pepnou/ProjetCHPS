#include "main.hpp"

using namespace cv;
using namespace std;




int main(int argc, char** argv)
{
	mpf_t x, y, w, h;

	// mpf_init_set_d( x, -0.5);
	// mpf_init_set_d( y, 0.0);
	// mpf_init_set_d( w, 3);
	// mpf_init_set_d( h, 2.0);

	mpf_init_set_d( x, -1.5);
	mpf_init_set_d( y, 0.0);
	mpf_init_set_d( w, 0.5);
	mpf_init_set_d( h, 0.28125);

	Mandelbrot M( x, y, w, h, 1920, 1080, 4, 200);
	// Mandelbrot M( x, y, w, h, 240, 135, 4, 200);

	M.escapeSpeedCalc();
	M.draw();
	M.save();

	mpf_clears( x, y, w, h, NULL);
	exit(0);
}