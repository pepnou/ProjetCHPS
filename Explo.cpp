#include <mpi.h>
#include <cstdlib>
#include <iostream>
#include <boost/program_options.hpp>


#include "workHandler.hpp"
#include "mpmc.hpp"
#include "mandelbrot.hpp"

#define KILL 69
#define MPMC_SIZE 50000


int Mandelbrot::surEchantillonage;
int Mandelbrot::im_width;
int Mandelbrot::im_height;
int Mandelbrot::color;
char* Mandelbrot::rep;
keyed_char* Mandelbrot::top10 = new keyed_char[11];
Mpmc* Mandelbrot::mpmc;



void getExploOptions(int argc, char** argv, Mpmc* mpmc, bool& verbose)
{
	// default exploration options
		std::string x("-0.5"), y("0"), w("3"), h("2");
		int enough = 5;

		int default_param[5];
		// largeur
		default_param[0] = 48;
		// hauteur
		default_param[1] = 27;
		// sur echantillonage
		default_param[2] = 1;
		// couleur
		default_param[3] = RAINBOW;

	try
	{
		boost::program_options::options_description generic("Generic options");
		generic.add_options()
			("help", ": describe arguments")
			("verbose,v", ": be verbose")
			("config,c", ": only generate config file");

		boost::program_options::options_description config("Configuration");
		config.add_options()
			("im-width,w", boost::program_options::value< int >(), ": width of the generated images")
			("im-height,h", boost::program_options::value< int >(), ": height of the generated images")
			("Xposition,X", boost::program_options::value< std::string >(), ": abscissa of the center of the current fractal zone in the complex plane, should be between -2 and 2")
			("Yposition,Y", boost::program_options::value< std::string >(), ": ordinate of the center of the current fractal zone in the complex plane, should be between -2 and 2")
			("width,W", boost::program_options::value< std::string >(), ": width of the current fractal zone in the complex plane, see --help-dimension")
			("height,H", boost::program_options::value< std::string >(), ": height of the current fractal zone in the complex plane, see --help-dimension")
			("color,C", boost::program_options::value< int >(), ": the color algorithm used, see --help-color")
			("enough,E", boost::program_options::value< int >(), ": the maximum number of dichotomical division before stoping")
			("super-sampling,S", boost::program_options::value< int >(), ": the maximum number of points calculated per pixel");

		boost::program_options::options_description hidden("Hidden options");
		hidden.add_options()
			("help-color", "")
			("help-thread", "")
			("help-dimension", "");

		boost::program_options::options_description cmdline_options;
		cmdline_options.add(generic).add(config).add(hidden);

		boost::program_options::options_description config_file_options;
		config_file_options.add(config);

		boost::program_options::options_description visible;
		visible.add(generic).add(config);

		boost::program_options::variables_map vm;

		std::ifstream ifs("ExploConfig.cfg");
		store(parse_config_file(ifs, config_file_options), vm);
		notify(vm);

		if (vm.count("Xposition"))
		{
			x = vm.count("Xposition");
		}
		if (vm.count("Yposition"))
		{
			y = vm.count("Yposition");
		}
		
		if (vm.count("im-width"))
		{
			default_param[0] = vm["im-width"].as<int>();
		}
		
		if (vm.count("im-height"))
		{
			default_param[1] = vm["im-height"].as<int>();
		}
		if (vm.count("width"))
		{
			w = vm.count("width");
		}
		
		if (vm.count("height"))
		{
			h = vm.count("height");
		}
		
		if (vm.count("color"))
		{
			default_param[3] = vm["color"].as<int>();
		}
		
		if (vm.count("enough"))
		{
			enough = vm["enough"].as<int>();
		}
		
		if (vm.count("super-sampling"))
		{
			default_param[2] = vm["super-sampling"].as<int>();
		}
		
		boost::program_options::positional_options_description positional;
		boost::program_options::variables_map vm2;

		boost::program_options::store(boost::program_options::command_line_parser(argc, argv)
			.options(cmdline_options)
			.positional(positional)
			.run(),
			vm2);

		boost::program_options::notify(vm2);

		if (vm2.count("help"))
		{
			std::cout << visible << "\n";
			exit(0);
		}
		
		if (vm2.count("Xposition"))
		{
			x = vm2.count("Xposition");
		}

		if (vm2.count("Yposition"))
		{
			y = vm2.count("Yposition");
		}
		
		if (vm2.count("im-width"))
		{
			default_param[0] = vm2["im-width"].as<int>();
		}
		
		if (vm2.count("im-height"))
		{
			default_param[1] = vm2["im-height"].as<int>();
		}

		if (vm2.count("width"))
		{
			w = vm2.count("width");
		}
		
		if (vm2.count("height"))
		{
			w = vm2.count("height");
		}
		
		if (vm2.count("color"))
		{
			default_param[3] = vm2["color"].as<int>();
		}
		
		if (vm2.count("enough"))
		{
			enough = vm2["enough"].as<int>();
		}
		
		if (vm2.count("super-sampling"))
		{
			default_param[3] = vm2["super-sampling"].as<int>();
		}

		if (vm2.count("verbose"))
		{
			verbose = true;
		}
		
		if (vm2.count("help-color"))
		{
			std::cout<< "This option allows you to choose from the three currently implemented coloring algorithm, it should only take a number between 1 et 3" << std::endl
				<< "\t1: This algorithm take the escape speed time of a point (or multiple if you are using the super sampling), reduce it between 0 et 360 using a modulo and then use it as hue in the hsb color representation" << std::endl
				<< "\t2:This algorithm take the escape speed time of a point (or multiple if you are using the super sampling), reduce it between 0 et 360 using a modulo and then cut it in 4 phase to allow to go from blue, to black, to yellow, to white and to blue again" << std::endl
				<< "\t3: This algorithm take the escape speed time of a point (or multiple if you are using the super sampling), calculate three value using alpha*cos(escape_speed) or alpha*sin(escape_speed) and then use them as red, green and blue components" << std::endl
				<< std::endl;
			exit(0);
		}

		if (vm2.count("help-thread"))
		{
			std::cout<< "This option allows you to choose the maximum number of thread created by the program." << std::endl
				<< "Note that one more thread is created to explore the fractal and create tasks for the other thread." << std::endl
				<< "If you want to use all your possible threds, use -1." << std::endl
				<< "There is no current way to only use one thread." << std::endl
				<< std::endl;
			exit(0);
		}

		if (vm2.count("help-dimension"))
		{
			std::cout<< "This option allows yout to choose the width and the height of the current fractal zone in the complex plane." << std::endl
				<< "Note that if the ratio of the image is different than the one of the complex plan, generated images will be out of shape." << std::endl
				<< std::endl;
			exit(0);
		}

		std::ofstream ofs("ExploConfig.cfg",std::ofstream::trunc);
		ofs << "Xposition=" << x << std::endl
			<< "Yposition=" << y << std::endl
			<< "im-width=" << default_param[0] << std::endl
			<< "im-height=" << default_param[1] << std::endl
			<< "width=" << w << std::endl
			<< "height=" << h << std::endl
			<< "color=" << default_param[3] << std::endl
			<< "enough=" << enough << std::endl
			<< "super-sampling=" << default_param[2] << std::endl;
 
		if (vm2.count("config"))
		{
			MPI_Abort(MPI_COMM_WORLD, 0);
		}
	}
	catch(std::exception& E)
	{
		std::cout << E.what() << std::endl;
		MPI_Abort(MPI_COMM_WORLD, 1);
	}


	std::stringstream r;
	r.str("");
	std::time_t t = std::time(0);
	std::tm* now = std::localtime(&t);
	
	r << std::setfill('0') << std::right << "../Img/" << now->tm_year + 1900 << "-" << std::setw(2) << now->tm_mon + 1 << "-" << std::setw(2) << now->tm_mday << "_" << std::setw(2) << now->tm_hour << ":" << std::setw(2) << now->tm_min << ":" << std::setw(2) << now->tm_sec;

	Mandelbrot::rep = new char[r.str().size() + 1]();
	strcpy(Mandelbrot::rep, r.str().c_str());
	Mandelbrot::rep[r.str().size()] = '\0';

	default_param[4] = r.str().size() + 1;


	MPI_Bcast(default_param, 5, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(Mandelbrot::rep, default_param[4], MPI_CHAR, 0, MPI_COMM_WORLD);
}

void receiveExploOptions()
{
	int default_param[5];
	MPI_Bcast(default_param, 5, MPI_INT, 0, MPI_COMM_WORLD);

	Mandelbrot::im_width = default_param[0];
	Mandelbrot::im_height = default_param[1];
	Mandelbrot::surEchantillonage = default_param[2];
	Mandelbrot::color = default_param[3];
	Mandelbrot::rep = new char[default_param[4]];

	MPI_Bcast(Mandelbrot::rep, default_param[4], MPI_CHAR, 0, MPI_COMM_WORLD);
}


char* getWork()
{
	int rank, pere, fd, fg;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);


	char* work = NULL;

	if(Mandelbrot::mpmc->pop(rank, work) == MPMC_SUCCES);
	else if (Mandelbrot::mpmc->pop(fg, work) == MPMC_SUCCES);
	else if (Mandelbrot::mpmc->pop(pere, work) == MPMC_SUCCES);
	else if (Mandelbrot::mpmc->pop(fd, work) == MPMC_SUCCES);
	
	return work;
}


int main(int argc, char** argv)
{
	MPI_Init(&argc, &argv);
	
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);


	Mandelbrot::mpmc = new Mpmc(MPMC_SIZE);


	bool verbose;
	
	if(rank == 0)
		getExploOptions(argc, argv, Mandelbrot::mpmc, verbose);
	else
		receiveExploOptions();
	
	uint64_t tick = rdtsc();

	Mandelbrot* M;
	char* work;

	//ALORS OUI MAIS NON HEIN
	int* voisin;
	
	while(1)
	{
		work = NULL;
		while((work = getWork()))
		{
			M = new Mandelbrot(work);
			M->dichotomie3();
			delete M;
			delete [] work;
		}

		//si on a pas de fils
		if(voisin[1] == -1)
			setState('s');
		//si on a juste 1 fils et qu'il dort (gauche)
		if(voisin[2] == -1)
			if(getState(voisin[1]) == 's')
				setState('s');

		//si on a juste 1 fils et qu'il dort (droite)
		if(voisin[2] == -1)
			if(getState(voisin[1]) == 's')
				setState('s');

		//si on a 2 fils et qu'ils dorment tout les 2
		if(getState(voisin[1]) == 's' && getState(voisin[2] == 's'))
			setState('s');
		if(voisin[0] == -1 && getState(rank) == 's')
			for (int i = 0; voisin[i] != -1; ++i)
			{
				MPI_Send(0, 0, MPI_BYTE, voisin[i], KILL, MPI_COMM_WORLD);
				break;
			}
		//usleep(100);
	}

	MPI_Recv(0, 0, MPI_BYTE, voisin[0], KILL, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

	if(voisin[1] == -1 && voisin[2] == -1)
		//send top 10 to daddy
		{
	    std::stringstream list("");
	    for( int i = 0;i < 10 && Mandelbrot::top10[i].val != NULL; i++)
	    {
	        if(i == 0)
	            list << Mandelbrot::top10[i].key << "|" << Mandelbrot::top10[i].val;
	        else
	            list << "|" << Mandelbrot::top10[i].key << "|" << Mandelbrot::top10[i].val;
	    }
	    MPI_Send(list.str().c_str(), list.str().size(), MPI_CHAR, 0, LIST_SEND, MPI_COMM_WORLD);
	    }

		//MPI_Send(Mandelbrot::top10, 1, MPI_CHAR, voisin[0], 0, MPI_COMM_WORLD);
	else
		for (int i = 1; voisin[i] != -1; ++i)
		{
			for(int i = 1; i < size; i++)
		    {
		    	//receive top 10 from child
		        MPI_Probe(i, LIST_SEND, MPI_COMM_WORLD, &status);
		        MPI_Get_count(&status, MPI_CHAR, &count);
		        buf = new char[count+1]();
		        MPI_Recv(buf, count, MPI_CHAR, status.MPI_SOURCE, LIST_SEND, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		        buf[count] = '\0';

		        tmp = strtok(buf, "|");
		        while(tmp)
		        {
		            key = atof(tmp);

		            tmp = strtok(NULL, "|");
		            val = new char[strlen(tmp) + 1]();
		            strcpy(val, tmp);
		            val[strlen(tmp)] = '\0';
		        
		            insert_top10(key, val);

		            tmp = strtok(NULL, "|");
		        }
		    }
		}
		if(voisin[0] == -1)
		{
			std::stringstream file_name("");
		    file_name << Mandelbrot::rep << "/Coordinates.txt";
		    
		    FILE* f2 = fopen(file_name.str().c_str(), "w+");
		    for(int i = 0; i < 10 && Mandelbrot::top10[i].val != NULL; i++)
		    {
		         fprintf(f2, "0:%s:2\n", Mandelbrot::top10[i].val);
		    }
		    fclose(f2);
		}
		else   
			MPI_Send(Mandelbrot::top10, 1, MPI_CHAR, voisin[0], 0, MPI_COMM_WORLD);

	if(rank == 0 && verbose == true)
		std::cout << "Time spend in cycle : " << rdtsc() - tick << std::endl;
	
	delete Mandelbrot::mpmc;

	MPI_Finalize();

	std::exit(0);
}


