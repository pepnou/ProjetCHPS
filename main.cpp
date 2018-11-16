#include "main.hpp"

using namespace cv;
using namespace std;

void ntm(void* tg)
{}

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
	const int NBR_THREADS = std::thread::hardware_concurrency();
	MyThreads* MT = new MyThreads(NBR_THREADS);
	Mpmc* mpmc = MT->getMpmc();


	int im_w = 960, im_h = 540, surech = 4, iteration = 20, enough = 3, color = 1;

	//coordonnée de debut de zoom et taille de la zone de zoomage
	mpf_init_set_d( x, -0.5);
	mpf_init_set_d( y, 0.0);
	mpf_init_set_d( w, 3);
	mpf_init_set_d( h, 2);


	Mandelbrot M( x, y, w, h, im_w, im_h, surech, iteration, color, mpmc);
	uint64_t tick;
	
	tick = rdtsc();
	M.dichotomie(enough);
	cout << rdtsc() - tick << endl;

	/*tick = rdtsc();
	M.escapeSpeedCalcThread2();
	M.draw();
	M.save();
	cout << rdtsc() - tick << endl;

	tick = rdtsc();
	M.escapeSpeedCalcThread3();
	M.draw();
	M.save();
	cout << rdtsc() - tick << endl;*/

	delete MT;
	mpf_clears( x, y, w, h, NULL);	
	exit(0);
}
