#include "main.hpp"

using namespace cv;
using namespace std;




int main(int argc, char** argv)
{	
	mpf_t x, y, w, h;
	int im_w = 38, im_h = 21, surech = 1, iteration = 100, enough = 1;

	// mpf_init_set_d( x, -1.5);
	// mpf_init_set_d( y, 0.0);
	// mpf_init_set_d( w, 0.5);
	// mpf_init_set_d( h, 0.28125);

	mpf_init_set_d( x, -0.5);
	mpf_init_set_d( y, 0.0);
	mpf_init_set_d( w, 3);
	mpf_init_set_d( h, 2);

	// Mandelbrot M( x, y, w, h, 1920, 1080, 4, 200); 
	// Mandelbrot M( x, y, w, h, 480, 270, 4, 200);
	Mandelbrot M( x, y, w, h, 240, 135, 1, 100);
	
	system("date");
	uint64_t tick = rdtsc();
	//M.dichotomie(enough);
	M.escapeSpeedCalcThread();

	system("date");
	cout << rdtsc() - tick << endl;
	
	M.draw2();
	M.save();
	
	mpf_clears( x, y, w, h, NULL);	
	exit(0);
}
