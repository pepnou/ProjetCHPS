#include "main.hpp"

using namespace cv;
using namespace std;

namespace po = boost::program_options;

int main(int argc, char** argv)
{
	int im_w = 1920, im_h = 1080, surech = 4, iteration = 100, enough = 1, color = RAINBOW;
	
	mpf_t x, y, w, h;
	mpf_init_set_d( x, -0.5);
	mpf_init_set_d( y, 0.0);
	mpf_init_set_d( w, 3);
	mpf_init_set_d( h, 2);

	try
	{
		po::options_description generic("Generic options");
		generic.add_options()
			("help,h", ": describe arguments")
			("verbose,v", "be verbose")
			;

		po::options_description config("Configuration");
		config.add_options()
			("im-width,W", po::value< int >(), ": width of the generated images")
			("im-height,H", po::value< int >(), ": height of the generated images")
			("Xposition,X", po::value< string >(), ": x position")
			("Yposition,Y", po::value< string >(), ": y position")
			("width,W", po::value< string >(), ": width of the fractal zone in the complex plane")
			("height,H", po::value< string >(), ": height of the fractal zone in the complex plane")
			("color,C", po::value< int >(), ": the color algorithm used, see --color-help")
			("enough,E", po::value< int >(), ": the maximum number of dichotomical division before stoping")
			("super-sampling,S", po::value< int >(), ": the maximum number of points calculated per pixel")
			;

		po::options_description hidden("Hidden options");
		hidden.add_options()
			("color-help,ch", ": describe arguments")
			;

		po::options_description cmdline_options;
		cmdline_options.add(generic).add(config).add(hidden);

		po::options_description config_file_options;
		config_file_options.add(config);

		po::options_description visible;
		visible.add(generic).add(config);


		po::variables_map vm;

        ifstream ifs("Config.cfg");
        store(parse_config_file(ifs, config_file_options), vm);
        notify(vm);

        if (vm.count("Xposition"))
        {
			mpf_set_str( x, vm["Xposition"].as<string>().c_str(), 10);
		}

		if (vm.count("Yposition"))
        {
			mpf_set_str( y, vm["Yposition"].as<string>().c_str(), 10);
		}
		
		if (vm.count("im-width"))
        {
			im_w = vm["im-width"].as<int>();
		}
		
		if (vm.count("im-height"))
        {
			im_h = vm["im-height"].as<int>();
		}

		if (vm.count("width"))
        {
			mpf_set_str( w, vm["width"].as<string>().c_str(), 10);
		}
		
		if (vm.count("height"))
        {
			mpf_set_str( h, vm["height"].as<string>().c_str(), 10);
		}
		
		if (vm.count("color"))
        {
			color = vm["color"].as<int>();
		}
		
		if (vm.count("enough"))
        {
			enough = vm["enough"].as<int>();
		}
		
		if (vm.count("super-sampling"))
        {
			surech = vm["super-sampling"].as<int>();
		}
		
		/*po::positional_options_description positional;

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
	}
	catch(std::exception& E)
	{
	    std::cout << E.what() << std::endl;
	}
	
	/*mpf_t x, y, w, h;
	const int NBR_THREADS = std::thread::hardware_concurrency();
	MyThreads* MT = new MyThreads(NBR_THREADS);
	//MyThreads* MT = new MyThreads(1);
	Mpmc* mpmc = MT->getMpmc();


	int im_w = 960, im_h = 540, surech = 3, iteration = 100, enough = 2, color = RAINBOW;

	//coordonnée de debut de zoom et taille de la zone de zoomage
	mpf_init_set_d( x, -0.5);
	mpf_init_set_d( y, 0.0);
	mpf_init_set_d( w, 3);
	mpf_init_set_d( h, 2);

	Mandelbrot M( x, y, w, h, im_w, im_h, surech, iteration, color, mpmc);
	uint64_t tick;*/
	
	/*tick = rdtsc();
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
	
	/*delete MT;
	mpf_clears( x, y, w, h, NULL);*/
	exit(0);
}
