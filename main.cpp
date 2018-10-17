#include "main.hpp"

using namespace cv;
using namespace std;




int main(int argc, char** argv)
{	
	mpf_t x, y, w, h;

	// mpf_init_set_d( x, -1.5);
	// mpf_init_set_d( y, 0.0);
	// mpf_init_set_d( w, 0.5);
	// mpf_init_set_d( h, 0.28125);

	mpf_init_set_d( x, -0.5);
	mpf_init_set_d( y, 0.0);
	mpf_init_set_d( w, 3);
	mpf_init_set_d( h, 2);

	// Mandelbrot M( x, y, w, h, 1920, 1080, 4, 200); 
	Mandelbrot M( x, y, w, h, 480, 270, 4, 200);
	
	system("date");
	uint64_t tick = rdtsc();
	M.escapeSpeedCalcThread();
	system("date");
	cout << rdtsc() - tick << endl;
	
	
	M.draw();
	M.save();
	
	
	//~ int s = clock()/CLOCKS_PER_SEC, m, he;
	//~ he = s/3600;
	//~ m = (s - he*60)/60;
	//~ s = s - he*3600 - m*60;
	
	//~ cout<<he<<"h"<<m<<"min"<<s<<"sec"<<endl;
		
	mpf_clears( x, y, w, h, NULL);	
	exit(0);
}
