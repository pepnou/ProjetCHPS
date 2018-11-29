#include "main.hpp"

using namespace cv;
using namespace std;

namespace po = boost::program_options;

int main(int argc, char** argv)
{
	//mpf_set_default_prec(MAX_PREC);
	//10^-618

	//C'EST ICI QUE TU CHANGES LES PARAMETRES POUR CHANGER LE RESULTAT FINAL BONHOMME !
	int im_w = 1920, im_h = 1080, surech = 4, iteration = 100, enough = 1, color = RAINBOW;
	int nbt = thread::hardware_concurrency();
	bool verbose = false;
	
	mpf_t x, y, w, h;

	/*mpf_init_set_d( x, -0.5);
	mpf_init_set_d( y, 0.0);
	mpf_init_set_d( w, 3);
	mpf_init_set_d( h, 2);*/

	mpf_init_set_str( x, "-0.5", 10);
	mpf_init_set_str( y, "0e30", 10);
	mpf_init_set_str( w, "3", 10);
	mpf_init_set_str( h, "2", 10);

	try
	{
		po::options_description generic("Generic options");
		generic.add_options()
			("help", ": describe arguments")
			("verbose,v", ": be verbose")
			("config,c", ": only generate config file")
			;

		po::options_description config("Configuration");
		config.add_options()
			("im-width,w", po::value< int >(), ": width of the generated images")
			("im-height,h", po::value< int >(), ": height of the generated images")
			("Xposition,X", po::value< string >(), ": abscissa of the center of the current fractal zone in the complex plane, should be between -2 and 2")
			("Yposition,Y", po::value< string >(), ": ordinate of the center of the current fractal zone in the complex plane, should be between -2 and 2")
			("width,W", po::value< string >(), ": width of the current fractal zone in the complex plane, see --help-dimension")
			("height,H", po::value< string >(), ": height of the current fractal zone in the complex plane, see --help-dimension")
			("color,C", po::value< int >(), ": the color algorithm used, see --help-color")
			("enough,E", po::value< int >(), ": the maximum number of dichotomical division before stoping")
			("super-sampling,S", po::value< int >(), ": the maximum number of points calculated per pixel")
			("thread,T", po::value< int >(), ": the maximum number of thread, see --help-thread")
			;

		po::options_description hidden("Hidden options");
		hidden.add_options()
			("help-color", "")
			("help-thread", "")
			("help-dimension", "")
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
			int prec = ceil(vm["Xposition"].as<string>().length()*log(10)/log(2));
			prec = (prec%64 != 0)?(prec/64)*64+64:(prec/64)*64;
			prec = (prec < 64)?64:prec;
			//prec = (prec > MAX_PREC)?MAX_PREC:prec;

			mpf_set_prec_raw( x, prec);
			mpf_set_prec( x, prec);

			mpf_set_str( x, vm["Xposition"].as<string>().c_str(), 10);
		}

		if (vm.count("Yposition"))
		{
			int prec = ceil(vm["Yposition"].as<string>().length()*log(10)/log(2));
			prec = (prec%64 != 0)?(prec/64)*64+64:(prec/64)*64;
			prec = (prec < 64)?64:prec;
			//prec = (prec > MAX_PREC)?MAX_PREC:prec;

			mpf_set_prec_raw( y, prec);
			mpf_set_prec( y, prec);

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
			// 2^x > 10^n
			// x > log2(10^n) = n*log2(10) = n*ln(10)/ln(2)

			int prec = ceil(vm["width"].as<string>().length()*log(10)/log(2));
			prec = (prec%64 != 0)?(prec/64)*64+64:(prec/64)*64;
			prec = (prec < 64)?64:prec;
			//prec = (prec > MAX_PREC)?MAX_PREC:prec;

			mpf_set_prec_raw( w, prec);
			mpf_set_prec( w, prec);

			mpf_set_str( w, vm["width"].as<string>().c_str(), 10);
		}
		
		if (vm.count("height"))
		{
			// 2^x > 10^n
			// x > log2(10^n) = n*log2(10) = n*ln(10)/ln(2)

			int prec = ceil(vm["height"].as<string>().length()*log(10)/log(2));
			prec = (prec%64 != 0)?(prec/64)*64+64:(prec/64)*64;
			prec = (prec < 64)?64:prec;
			//prec = (prec > MAX_PREC)?MAX_PREC:prec;

			mpf_set_prec_raw( w, prec);
			mpf_set_prec( h, prec);

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

		if (vm.count("thread"))
		{
			surech = vm["thread"].as<int>();
		}


		
		po::positional_options_description positional;

		po::variables_map vm2;

		po::store(po::command_line_parser(argc, argv)
			.options(cmdline_options)
			.positional(positional)
			.run(),
			vm2);

		po::notify(vm2);

		if (vm2.count("help"))
		{
			std::cout << visible << "\n";
			exit(0);
		}

		if (vm2.count("Xposition"))
		{
			int prec = ceil(vm2["Xposition"].as<string>().length()*log(10)/log(2));
			prec = (prec%64 != 0)?(prec/64)*64+64:(prec/64)*64;
			prec = (prec < 64)?64:prec;
			//prec = (prec > MAX_PREC)?MAX_PREC:prec;

			mpf_set_prec_raw( x, prec);
			mpf_set_prec( x, prec);

			mpf_set_str( x, vm2["Xposition"].as<string>().c_str(), 10);
		}

		if (vm2.count("Yposition"))
		{
			int prec = ceil(vm2["Yposition"].as<string>().length()*log(10)/log(2));
			prec = (prec%64 != 0)?(prec/64)*64+64:(prec/64)*64;
			prec = (prec < 64)?64:prec;
			//prec = (prec > MAX_PREC)?MAX_PREC:prec;

			mpf_set_prec_raw( y, prec);
			mpf_set_prec( y, prec);

			mpf_set_str( y, vm2["Yposition"].as<string>().c_str(), 10);
		}
		
		if (vm2.count("im-width"))
		{
			im_w = vm2["im-width"].as<int>();
		}
		
		if (vm2.count("im-height"))
		{
			im_h = vm2["im-height"].as<int>();
		}

		if (vm2.count("width"))
		{
			// 2^x > 10^n
			// x > log2(10^n) = n*log2(10) = n*ln(10)/ln(2)

			int prec = ceil(vm2["width"].as<string>().length()*log(10)/log(2));
			prec = (prec%64 != 0)?(prec/64)*64+64:(prec/64)*64;
			prec = (prec < 64)?64:prec;
			//prec = (prec > MAX_PREC)?MAX_PREC:prec;

			mpf_set_prec_raw( w, prec);
			mpf_set_prec( w, prec);

			mpf_set_str( w, vm2["width"].as<string>().c_str(), 10);
		}
		
		if (vm2.count("height"))
		{
			// 2^x > 10^n
			// x > log2(10^n) = n*log2(10) = n*ln(10)/ln(2)

			int prec = ceil(vm2["height"].as<string>().length()*log(10)/log(2));
			prec = (prec%64 != 0)?(prec/64)*64+64:(prec/64)*64;
			prec = (prec < 64)?64:prec;
			//prec = (prec > MAX_PREC)?MAX_PREC:prec;

			mpf_set_prec_raw( w, prec);
			mpf_set_prec( h, prec);

			mpf_set_str( h, vm2["height"].as<string>().c_str(), 10);
		}
		
		if (vm2.count("color"))
		{
			color = vm2["color"].as<int>();
		}
		
		if (vm2.count("enough"))
		{
			enough = vm2["enough"].as<int>();
		}
		
		if (vm2.count("super-sampling"))
		{
			surech = vm2["super-sampling"].as<int>();
		}

		if (vm2.count("thread"))
		{
			if(vm2["thread"].as<int>() == -1 || vm2["thread"].as<int>()>thread::hardware_concurrency())
				nbt = thread::hardware_concurrency();
			else
				nbt = vm2["thread"].as<int>();
		}

		if (vm2.count("verbose"))
		{
			verbose = true;
		}


		if (vm2.count("help-color"))
		{
			cout<< "This option allows you to choose from the three currently implemented coloring algorithm, it should only take a number between 1 et 3" << endl
				<< "\t1: This algorithm take the escape speed time of a point (or multiple if you are using the super sampling), reduce it between 0 et 360 using a modulo and then use it as hue in the hsb color representation" << endl
				<< "\t2:This algorithm take the escape speed time of a point (or multiple if you are using the super sampling), reduce it between 0 et 360 using a modulo and then cut it in 4 phase to allow to go from blue, to black, to yellow, to white and to blue again" << endl
				<< "\t3: This algorithm take the escape speed time of a point (or multiple if you are using the super sampling), calculate three value using alpha*cos(escape_speed) or alpha*sin(escape_speed) and then use them as red, green and blue components" << endl
				<< endl;
			exit(0);
		}

		if (vm2.count("help-thread"))
		{
			cout<< "This option allows you to choose the maximum number of thread created by the program." << endl
				<< "Note that one more thread is created to explore the fractal and create tasks for the other thread." << endl
				<< "If you want to use all your possible threds, use -1." << endl
				<< "There is no current way to only use one thread." << endl
				<< endl;
			exit(0);
		}

		if (vm2.count("help-dimension"))
		{
			cout<< "This option allows yout to choose the width and the height of the current fractal zone in the complex plane." << endl
				<< "Note that if the ratio of the image is different than the one of the complex plan, generated images will be out of shape." << endl
				<< endl;
			exit(0);
		}



		mp_exp_t e1, e2, e3, e4;
		char *char_width, *char_height, *char_x, *char_y;
		char tmpx[3] = {'0','.','\0'}, tmpy[3] = {'0','.','\0'};


		char_x = mpf_get_str( NULL, &e1, 10, 1000, x);
		if(char_x[0] == '-')
		{
			char_x[0] = '.';
			tmpx[0] = '-';
			tmpx[1] = '0';
		}
		char_y = mpf_get_str( NULL, &e2, 10, 1000, y);
		if(char_y[0] == '-')
		{
			char_y[0] = '.';
			tmpy[0] = '-';
			tmpy[1] = '0';
		}
		char_width = mpf_get_str( NULL, &e3, 10, 1000, w);
		char_height = mpf_get_str( NULL, &e4, 10, 1000, h);

		ofstream ofs("Config.cfg",ofstream::trunc);
		ofs << "Xposition=" << tmpx << char_x << "e" << e1 << endl
			<< "Yposition=" << tmpy << char_y << "e" << e2 << endl
			<< "im-width=" << im_w << endl
			<< "im-height=" << im_h << endl
			<< "width=" << "0." << char_width << "e" << e3 << endl
			<< "height=" << "0." << char_height << "e" << e4 << endl
			<< "color=" << color << endl
			<< "enough=" << enough << endl
			<< "super-sampling=" << surech << endl
			<< "thread=" << nbt << endl;

		if (vm2.count("config"))
		{
			exit(0);
		}
	}
	catch(std::exception& E)
	{
		std::cout << E.what() << std::endl;
	}


	if(mpf_get_prec(w) > mpf_get_prec(x))
	{
		mpf_set_prec_raw( x, mpf_get_prec(w));
		mpf_set_prec( x, mpf_get_prec(w));
	}

	if(mpf_get_prec(h) > mpf_get_prec(y))
	{
		mpf_set_prec_raw( y, mpf_get_prec(h));
		mpf_set_prec( y, mpf_get_prec(h));
	}


	MyThreads* MT = new MyThreads(nbt);
	Mpmc* mpmc = MT->getMpmc();
	Mandelbrot M( x, y, w, h, im_w, im_h, surech, iteration, color, mpmc);

	uint64_t tick;
	
	tick = rdtsc();
	M.dichotomie( enough, 0);
	if(verbose)cout <<"Time spend in cycle : "<< rdtsc() - tick << endl;
	
	delete MT;
	mpf_clears( x, y, w, h, NULL);
	exit(0);
}
