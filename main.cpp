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

	int im_w = 960, im_h = 540, surech = 4, iteration = 50, enough = 2, color = 1;

	mpf_init_set_d( x, -0.5);
	mpf_init_set_d( y, 0.0);
	mpf_init_set_d( w, 3);
	mpf_init_set_d( h, 2);

	Mandelbrot M( x, y, w, h, im_w, im_h, surech, iteration, color);
	uint64_t tick;
	
	/*tick = rdtsc();
	M.dichotomie(enough);
	cout << rdtsc() - tick << endl;*/

	tick = rdtsc();
	M.escapeSpeedCalcThread2();
	M.draw();
	M.save();
	cout << rdtsc() - tick << endl;

	tick = rdtsc();
	M.escapeSpeedCalcThread3();
	M.draw();
	M.save();
	cout << rdtsc() - tick << endl;
	

	
	mpf_clears( x, y, w, h, NULL);	
	exit(0);
}
