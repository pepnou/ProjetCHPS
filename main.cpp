#include "main.hpp"

using namespace cv;
using namespace std;




int main(int argc, char** argv)
{
	mpf_t x, y, w, h;

	int im_w = 120, im_h = 62, surech = 2, iteration = 100, enough = 5;

	// mpf_init_set_d( x, -1.5);
	// mpf_init_set_d( y, 0.0);
	// mpf_init_set_d( w, 0.5);
	// mpf_init_set_d( h, 0.28125);

	mpf_init_set_d( x, -0.5);
	mpf_init_set_d( y, 0.0);
	mpf_init_set_d( w, 3);
	mpf_init_set_d( h, 2);

	// Mandelbrot M( x, y, w, h, 1920, 1080, 4, 200); 
	// Mandelbrot M( x, y, w, h, 480, 270, 2, 100);

	Mandelbrot M( x, y, w, h, im_w, im_h, surech, iteration);

	
	
	system("date");
	uint64_t tick = rdtsc();

	//M.escapeSpeedCalcThread2();
	M.dichotomie(enough);
	
	system("date");
	cout << rdtsc() - tick << endl;
	

	//M.draw2();
	//M.save();
	
	mpf_clears( x, y, w, h, NULL);	
	exit(0);
}
