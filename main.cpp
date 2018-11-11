#include "main.hpp"

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
	/*namespace po = boost::program_options;
	
	po::options_description options("Options");
	options.add_options()
	("help,h", "describe arguments")
	("verbose,v", "be verbose");
	
	po::positional_options_description positional;

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv)
		.options(options)
		.positional(positional)
		.run(),
		vm);
	po::notify(vm);

	if (vm.count("help")) {
	std::cout << options << "\n";
	exit(0);
	}*/
	
	mpf_t x, y, w, h;

	int im_w = 480, im_h = 270, surech = 1, iteration = 50, enough = 2, color = 1;

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
	Mandelbrot M( x, y, w, h, im_w, im_h, surech, iteration, color);
	//M.escapeSpeedCalcThread();
	//M.draw();
	//M.save();

	
	// system("date");
	uint64_t tick = rdtsc();
	
	// M.escapeSpeedCalcThread2();
	M.dichotomie(enough);
	
	cout << rdtsc() - tick << endl;
	// system("date");
	
	// M.draw2();
	// M.save();
	
	mpf_clears( x, y, w, h, NULL);	
	exit(0);
}
