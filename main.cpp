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

	int i = 1;
	work wo, wooooooo;
	wo.arg = (void*)&i;
	wo.f = ntm;

	dbg
	Mpmc* mpmc = new Mpmc(50);
	dbg
	mpmc->push(wo);
	dbg
	mpmc->pop(&wooooooo);
	dbg

	cout<<*((int*)wooooooo.arg)<<endl;
	delete mpmc;

	/*int im_w = 480, im_h = 270, surech = 4, iteration = 100, enough = 3, color = 1;

	//coordonnée de debut de zoom et taille de la zone de zoomage
	mpf_init_set_d( x, -0.5);
	mpf_init_set_d( y, 0.0);
	mpf_init_set_d( w, 3);
	mpf_init_set_d( h, 2);


	Mandelbrot M( x, y, w, h, im_w, im_h, surech, iteration, color);
	uint64_t tick;
	
	tick = rdtsc();
	M.dichotomie(enough);
	cout << rdtsc() - tick << endl;*/

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

	
	//mpf_clears( x, y, w, h, NULL);	
	exit(0);
}
