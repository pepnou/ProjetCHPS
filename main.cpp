#include "main.hpp"

using namespace cv;
using namespace std;




int main(int argc, char** argv)
{
	mpf_t x, y, w, h;
	int im_w = 38, im_h = 21, surech = 1, iteration = 100, enough = 3;

	mpf_init_set_d( x, -0.5);
	mpf_init_set_d( y, 0.0);
	mpf_init_set_d( w, 3);
	mpf_init_set_d( h, 2.0);

	

	Mandelbrot M( x, y, w, h, 240, 135, 4, 200);

	M.dichotomie(enough);
	
	/*M.escapeSpeedCalc();
	M.draw();
	M.save();*/
	
	cout<<clock()/CLOCKS_PER_SEC<<endl;
	
	mpf_clears( x, y, w, h, NULL);
		
	exit(0);
}
