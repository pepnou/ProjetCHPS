#include <mpi.h>
#include <queue>
#include <string>
#include <string.h>
#include <boost/program_options.hpp>
#include <fstream>
#include <cmath>
#include <ctime>
#include <vector>
#include <unistd.h>
#include <map>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <stdint.h>


#include "mandelbrot.hpp"
#include "workHandler.hpp"
#include "matOp.hpp"
#include "rdtsc.hpp"

#define RAINBOW 1
#define JAUNE_BLEU 2
#define SINCOS 3

#define INFO_RQST 0
#define WORK_SEND 1
#define WORK_RQST 2
#define END 3
#define REP_SEND 4
#define LIST_SEND 5

//controle curseur
#define SAVEC "\e[s"
#define RESTC "\e[u"
#define DELLI "\e[K"


int Mandelbrot::surEchantillonage;
int Mandelbrot::im_width;
int Mandelbrot::im_height;
int Mandelbrot::color;
char* Mandelbrot::rep;
keyed_char* Mandelbrot::top10 = new keyed_char[11];

void init_top10()
{
    for(int i = 0; i < 11; i++)
    {
        Mandelbrot::top10[i].key = 0;
        Mandelbrot::top10[i].val = NULL;
    }
}


void insert_top10(double key, char* val)
{
    if(Mandelbrot::top10[10].val != NULL)
        delete [] Mandelbrot::top10[10].val;

    Mandelbrot::top10[10].key = key;
    Mandelbrot::top10[10].val = val;
    
    int iter = 10;
    double keyswap;
    char* valswap;
    while(iter>0)
    {
        if(Mandelbrot::top10[iter].key > Mandelbrot::top10[iter-1].key)
        {
            keyswap = Mandelbrot::top10[iter].key;
            Mandelbrot::top10[iter].key = Mandelbrot::top10[iter-1].key;
            Mandelbrot::top10[iter-1].key = keyswap;

            valswap = Mandelbrot::top10[iter].val;
            Mandelbrot::top10[iter].val = Mandelbrot::top10[iter-1].val;
            Mandelbrot::top10[iter-1].val = valswap;
            
            iter--;
        }
        else
            break;
    }
}

bool getExploOptions(int argc, char** argv, int* default_param, mpf_t x, mpf_t y, mpf_t w, mpf_t h, int& enough, bool& verbose, int size)
{
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
			int prec = ceil(vm["Xposition"].as<std::string>().length()*log(10)/log(2));
			prec = (prec%64 != 0)?(prec/64)*64+64:(prec/64)*64;
			prec = (prec < 64)?64:prec;
			mpf_set_prec( x, prec);
			mpf_set_str( x, vm["Xposition"].as<std::string>().c_str(), 10);
		}
		if (vm.count("Yposition"))
		{
			int prec = ceil(vm["Yposition"].as<std::string>().length()*log(10)/log(2));
			prec = (prec%64 != 0)?(prec/64)*64+64:(prec/64)*64;
			prec = (prec < 64)?64:prec;
			mpf_set_prec( y, prec);
			mpf_set_str( y, vm["Yposition"].as<std::string>().c_str(), 10);
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
                    // 2^x > 10^n
                    // x > log2(10^n) = n*log2(10) = n*ln(10)/ln(2)
                    int prec = ceil(vm["width"].as<std::string>().length()*log(10)/log(2));
                    prec = (prec%64 != 0)?(prec/64)*64+64:(prec/64)*64;
                    prec = (prec < 64)?64:prec;
                    mpf_set_prec( w, prec);
                    mpf_set_str( w, vm["width"].as<std::string>().c_str(), 10);
                }
		
		if (vm.count("height"))
		{
			// 2^x > 10^n
			// x > log2(10^n) = n*log2(10) = n*ln(10)/ln(2)
			int prec = ceil(vm["height"].as<std::string>().length()*log(10)/log(2));
			prec = (prec%64 != 0)?(prec/64)*64+64:(prec/64)*64;
			prec = (prec < 64)?64:prec;
			mpf_set_prec( h, prec);
			mpf_set_str( h, vm["height"].as<std::string>().c_str(), 10);
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
			int prec = ceil(vm2["Xposition"].as<std::string>().length()*log(10)/log(2));
			prec = (prec%64 != 0)?(prec/64)*64+64:(prec/64)*64;
			prec = (prec < 64)?64:prec;
			mpf_set_prec( x, prec);
			mpf_set_str( x, vm2["Xposition"].as<std::string>().c_str(), 10);
		}
		if (vm2.count("Yposition"))
		{
			int prec = ceil(vm2["Yposition"].as<std::string>().length()*log(10)/log(2));
			prec = (prec%64 != 0)?(prec/64)*64+64:(prec/64)*64;
			prec = (prec < 64)?64:prec;
			mpf_set_prec( y, prec);
			mpf_set_str( y, vm2["Yposition"].as<std::string>().c_str(), 10);
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
			// 2^x > 10^n
			// x > log2(10^n) = n*log2(10) = n*ln(10)/ln(2)
			int prec = ceil(vm2["width"].as<std::string>().length()*log(10)/log(2));
			prec = (prec%64 != 0)?(prec/64)*64+64:(prec/64)*64;
			prec = (prec < 64)?64:prec;
			mpf_set_prec( w, prec);
			mpf_set_str( w, vm2["width"].as<std::string>().c_str(), 10);
		}
		
		if (vm2.count("height"))
		{
			// 2^x > 10^n
			// x > log2(10^n) = n*log2(10) = n*ln(10)/ln(2)
			int prec = ceil(vm2["height"].as<std::string>().length()*log(10)/log(2));
			prec = (prec%64 != 0)?(prec/64)*64+64:(prec/64)*64;
			prec = (prec < 64)?64:prec;
			mpf_set_prec( h, prec);
			mpf_set_str( h, vm2["height"].as<std::string>().c_str(), 10);
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
		std::ofstream ofs("ExploConfig.cfg",std::ofstream::trunc);
		ofs << "Xposition=" << tmpx << char_x << "e" << e1 << std::endl
			<< "Yposition=" << tmpy << char_y << "e" << e2 << std::endl
			<< "im-width=" << default_param[0] << std::endl
			<< "im-height=" << default_param[1] << std::endl
			<< "width=" << "0." << char_width << "e" << e3 << std::endl
			<< "height=" << "0." << char_height << "e" << e4 << std::endl
			<< "color=" << default_param[3] << std::endl
			<< "enough=" << enough << std::endl
			<< "super-sampling=" << default_param[2] << std::endl;
 
		if (vm2.count("config"))
		{
			for(int i = 1; i < size; i++)
			MPI_Send(NULL, 0, MPI_INT, i, END, MPI_COMM_WORLD);
			return false;
		}
	}
	catch(std::exception& E)
	{
		std::cout << E.what() << std::endl;
	}

	if(mpf_get_prec(w) > mpf_get_prec(x))
	{
	    mpf_set_prec( x, mpf_get_prec(w));
	}
	if(mpf_get_prec(h) > mpf_get_prec(y))
	{
	    mpf_set_prec( y, mpf_get_prec(h));
	}



	std::stringstream r;
    r.str("");
    std::time_t t = std::time(0);
    std::tm* now = std::localtime(&t);
    
    r << std::setfill('0') << std::right << "../Img/" << now->tm_year + 1900 << "-" << std::setw(2) << now->tm_mon + 1 << "-" << std::setw(2) << now->tm_mday << "_" << std::setw(2) << now->tm_hour << ":" << std::setw(2) << now->tm_min << ":" << std::setw(2) << now->tm_sec;


    Mandelbrot::rep = new char[r.str().size()]();
    strcpy(Mandelbrot::rep, r.str().c_str());

    for(int i = 1; i < size; i++)
        MPI_Send(Mandelbrot::rep, strlen(Mandelbrot::rep), MPI_CHAR, i, REP_SEND, MPI_COMM_WORLD);

    MPI_Bcast(default_param, 4, MPI_INT, 0, MPI_COMM_WORLD);

    return true;
}

bool receiveExploOptions()
{
	MPI_Status status;
    int count;

    MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    if(status.MPI_TAG == END)
        return false;

    MPI_Get_count(&status, MPI_CHAR, &count);
    Mandelbrot::rep = new char[count + 1]();
    MPI_Recv(Mandelbrot::rep, count, MPI_CHAR, 0, REP_SEND, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    Mandelbrot::rep[count] = '\0';

    int default_param[4];
    MPI_Bcast(default_param, 4, MPI_INT, 0, MPI_COMM_WORLD);

    Mandelbrot::im_width = default_param[0];
    Mandelbrot::im_height = default_param[1];
    Mandelbrot::surEchantillonage = default_param[2];
    Mandelbrot::color = default_param[3];

    return true;
}

void fill_work(std::queue<char*> *work, mpf_t x, mpf_t y, mpf_t w, mpf_t h, int enough, std::vector<int> divs, int size, int depth)
{
	mpf_t equalz;
    mpf_init(equalz);	

	std::vector<int> divs_cpy = divs;
	int prec = mpf_get_prec(x);

	for(int i = divs.size() - 1; i >= 0; i--)
    {
        int n_prec = prec + ceil(log(divs.at(i))/log(2));
        mpf_t temp, delta_x, delta_y;

        mpf_t* tab_x = new mpf_t[divs.at(i)];
        mpf_t* tab_y = new mpf_t[divs.at(i)];

        if(mpf_get_prec(x)>=mpf_get_prec(y))
            mpf_init2(temp, mpf_get_prec(x) + n_prec/64);
        else
            mpf_init2(temp, mpf_get_prec(y) + n_prec/64);

        mpf_init2(delta_x, mpf_get_prec(w));
        mpf_init2(delta_y, mpf_get_prec(h));

        //initialise chacun des elements du tableau avant de pouvoir s'en servir
        for (int init = 0; init < divs.at(i); init++)
        {
                mpf_init2(tab_x[init], mpf_get_prec(x) + n_prec/64);
                mpf_init2(tab_y[init], mpf_get_prec(y) + n_prec/64);
        }

        n_prec %= 64;


        //calcul de delta_x, la distance entre de nouveaux points en x
        mpf_div_ui(delta_x, w, divs.at(i));
        //calcul de delta_y, la distance entre de nouveaux points en y
        mpf_div_ui(delta_y, h, divs.at(i));


        //tab_x[0] = pos_x - width/2 + width/2*divs.at(i)
        mpf_div_ui(temp, w, 2*divs.at(i));
        mpf_add(tab_x[0], x, temp);
        mpf_div_ui(temp, w, 2);
        mpf_sub(tab_x[0], tab_x[0], temp);

        //tab_y[0] = pos_y - height/2 + height/2*divs.at(i)
        mpf_div_ui(temp, h, 2*divs.at(i));
        mpf_add(tab_y[0], y, temp);
        mpf_div_ui(temp, h, 2);
        mpf_sub(tab_y[0], tab_y[0], temp);

        for (int c = 1; c < divs.at(i); c++)
        {
            //tab_x[c] = tab_x[0] + c*delta_x
            mpf_mul_ui(temp, delta_x, c);
            mpf_add(tab_x[c], tab_x[0], temp);
            
            
            mpf_set_prec(equalz, mpf_get_prec(tab_y[c]));
            //tab_y[c] = tab_y[0] + c*delta_y
            mpf_mul_ui(temp, delta_y, c);
            mpf_add(tab_y[c], tab_y[0], temp);

            mpf_abs(equalz, tab_y[c]);
            mpf_div(equalz, equalz, h);
            if(mpf_cmp_d(equalz, 0.00001) < 0)
                mpf_set_ui(tab_y[c], 0);
        }

        for (int x = 0; x < divs.at(i); x++)
        {
            for (int y = 0; y < divs.at(i); y++)
            {
                if(mpf_cmp_ui(tab_y[y], 0) <= 0)
                {
                    if(depth == 0)
                    {
                    	char* buf = create_work(enough - 1, tab_x[x], tab_y[y], delta_x, delta_y, divs_cpy);
                    	work->push(buf);
                    }
                	else
                		fill_work(work, tab_x[x], tab_y[y], delta_x, delta_y, enough - 1, divs_cpy, size, depth - 1);
                }
            }
        }

        //delete chacun des element du tableau avant pour pouvoir virer le tableau
        for (int del = 0; del < divs.at(i); del++)
        {
            mpf_clear(tab_x[del]);
            mpf_clear(tab_y[del]);
        }

        delete [] tab_x;
        delete [] tab_y;
        mpf_clears(temp, delta_x, delta_y, NULL);

        divs_cpy.pop_back();
    }
}

void init_work(std::queue<char*> *work, mpf_t x, mpf_t y, mpf_t w, mpf_t h, int enough, std::vector<int> divs, int size)
{
	char* buf;
	/*buf = create_work(enough, x, y, w, h, divs);
    work->push(buf);*/

    int depth;
    long nb_image;
    for(depth = 0; depth < enough && nb_image < size - 1; depth++)
    {
    	nb_image = 0;
    	for(int i = 0; i <= depth; i++)
    	{
    		nb_image += pow(4, depth)*pow(9, i);
    	}
    }

    buf = create_work(depth, x, y, w, h, divs);
    work->push(buf);
    fill_work(work, x,  y, w, h, enough,divs, size, depth);
}

void handler(int argc, char** argv)
{
	init_top10();

	MPI_Status status;
	int size, rank, count;
	char* buf;

	std::queue<int> *waiting = new std::queue<int>();
	std::queue<char*> *work = new std::queue<char*>();


	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);


	std::vector<int> divs;
	divs.push_back(2);
	divs.push_back(3);

	int default_param[4];
	//PARAMETRES PAR DEFAULT, A NE PAS CHANGER
	// largeur
	default_param[0] = 48;
	// hauteur
	default_param[1] = 27;
	// sur echantillonage
	default_param[2] = 1;
	// couleur
	default_param[3] = RAINBOW;



	int enough = 1;
	bool verbose = false;

	mpf_t x, y, w, h;
	mpf_inits( x, y, w, h, NULL);

	mpf_set_d( x, -0.5);
	mpf_set_d( y, 0.0);
	mpf_set_d( w, 3);
	mpf_set_d( h, 2);


	if(!getExploOptions( argc, argv, default_param, x, y, w, h, enough, verbose, size))
		return;
    

    std::stringstream cmd("");
    cmd << "mkdir -p " << Mandelbrot::rep;
    int ret = system(cmd.str().c_str());
    if(ret == -1 || ret == 127)
    {
        perror("system");
        MPI_Abort(MPI_COMM_WORLD, 43);
    }


    init_work(work, x, y, w, h, enough, divs, size);
    



    int img_count = 0, info[2];
    int flag;


    unsigned long images_a_faire = 0, images_faites = 0, images_faites_recv;
    double pourcentage_images_faites;
    for(int k = 0; k <= enough; k++)
    {
        for(int j = 0; j <= k; j++)
        {
            images_a_faire += pow(4, j) * pow(9, k-j);
        }
    }

    std::cout << std::endl << SAVEC;

    uint64_t tick = rdtsc();
    while(1)
    {
        while(1)
        {
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
            if(flag)
                break;
            else
                usleep(100);
        }

        if (status.MPI_TAG == INFO_RQST)
        {
            MPI_Recv(&images_faites_recv, 1, MPI_LONG, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            info[0] = (int)work->size();
            info[1] = img_count;
            MPI_Send(info, 2, MPI_INT, status.MPI_SOURCE, INFO_RQST, MPI_COMM_WORLD);
            img_count++;
            
             
            images_faites += images_faites_recv;
            pourcentage_images_faites = (double)images_faites / images_a_faire;
            printf( RESTC DELLI "% 3.15lf%%", pourcentage_images_faites*100);
            fflush(stdout);
        } 
        else if (status.MPI_TAG == WORK_SEND) 
        {
            MPI_Get_count(&status, MPI_CHAR, &count);
            buf = new char[count+1]();
            MPI_Recv(buf, count, MPI_CHAR, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            buf[count] = '\0';
            // TODO meh ... tout sauf efficace .... a changer ...
            
            if(waiting->size() > 0)
            {

                MPI_Send(buf, count, MPI_CHAR, waiting->front(), WORK_SEND, MPI_COMM_WORLD);
                waiting->pop();
            }
            else
                work->push(buf);
        }
        else if (status.MPI_TAG == WORK_RQST)
        {
            MPI_Recv(NULL, 0, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if(work->size() > 0)
            {
                char* msg = work->front();
                MPI_Send(msg, strlen(msg), MPI_CHAR, status.MPI_SOURCE, WORK_SEND, MPI_COMM_WORLD);
                work->pop();
            }
            else
            {
                waiting->push(status.MPI_SOURCE);
                if(waiting->size() == (unsigned int)size - 1)
                {
                    for(int i = 1; i < size; i++)
                        MPI_Send(NULL, 0, MPI_INT, i, END, MPI_COMM_WORLD);

                    delete work;
                    delete waiting;

                    break;
                }
            }
        }
    }

    double key;
    char *val, *tmp;

    for(int i = 1; i < size; i++)
    {
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

    std::stringstream file_name("");
    file_name << Mandelbrot::rep << "/Coordinates.txt";
    
    FILE* f2 = fopen(file_name.str().c_str(), "w+");
    for(int i = 0; i < 10 && Mandelbrot::top10[i].val != NULL; i++)
    {
        fprintf(f2, "0:%s:2\n", Mandelbrot::top10[i].val);
    }
    fclose(f2);


    std::cout << std::endl;

    if(verbose)
    {
        std::cout <<"Time spend in cycle : "<< rdtsc() - tick << std::endl;
        FILE* f = fopen("log.txt", "a");
        fprintf(f, "%d %lu\n", size, rdtsc() - tick);
        fclose(f);
    }
}

void worker(int argc, char** argv)
{
    init_top10();

    MPI_Status status;
    int count;
    char* buf;

    if(!receiveExploOptions())
    	return;

    while(1)
    {
        MPI_Send(NULL, 0, MPI_INT, 0, WORK_RQST, MPI_COMM_WORLD);
        MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if(status.MPI_TAG == WORK_SEND)
        {
            MPI_Get_count(&status, MPI_CHAR, &count);
            buf = new char[count + 1]();
            
            MPI_Recv(buf, count, MPI_CHAR, 0, WORK_SEND, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            buf[count] = '\0';

            Mandelbrot* m = new Mandelbrot(buf);

            delete [] buf;

            m->dichotomie3();

            delete m;

        }
        else if(status.MPI_TAG == END)
        {
            break;
        }
    }

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

char* create_work(int enough, mpf_t x, mpf_t y, mpf_t w, mpf_t h, std::vector<int> divs)
{
    std::stringstream r("");
    
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
    
    r.str("");
    r << enough << ":" << tmpx << char_x << "e" << e1 << ":" << tmpy << char_y << "e" << e2 << ":" << "0." << char_width << "e" << e3 << ":" << "0." << char_height << "e" << e4;

    for(unsigned int i = 0; i < divs.size(); i++)
        r << ":" << divs.at(i);

    char* res = new char[r.str().size() + 1]();
    strcpy(res, r.str().c_str());
    res[r.str().size()] = '\0';
    
    return res;
}

char* create_work2(mpf_t x, mpf_t y, mpf_t w, mpf_t h)
{
    std::stringstream r("");
    
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
    
    r.str("");
    r << tmpx << char_x << "e" << e1 << ":" << tmpy << char_y << "e" << e2 << ":" << "0." << char_width << "e" << e3 << ":" << "0." << char_height << "e" << e4;

    char* res = new char[r.str().size() + 1]();
    strcpy(res, r.str().c_str());
    res[r.str().size()] = '\0';
    
    return res;
}

void getHandlerInfo(bool& needwork, int& img_num, long images_faites)
{
    MPI_Send(&images_faites, 1, MPI_LONG, 0, INFO_RQST, MPI_COMM_WORLD);

    int size, info[2];
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    MPI_Recv(info, 2, MPI_INT, 0, INFO_RQST, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    if(info[0] < size)
        needwork = true; 
    else
        needwork = false;

    img_num = info[1];
}

void sendWork(char* buf)
{
    MPI_Ssend(buf, strlen(buf), MPI_CHAR, 0, WORK_SEND, MPI_COMM_WORLD);
}

bool getGenOptions(int argc, char** argv, int* default_param, char* &file, bool& verbose, int size, int& blocHeight)
{
	file = NULL;
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
			("file,f", boost::program_options::value< std::string>(), ": file containing positions of places in the fractal used to genrate images")
			("color,C", boost::program_options::value< int >(), ": the color algorithm used, see --help-color")
			("super-sampling,S", boost::program_options::value< int >(), ": the maximum number of points calculated per pixel")
            ("blocHeight,H", boost::program_options::value< int >(), ": the height of the blocs the images are divided into");

		boost::program_options::options_description hidden("Hidden options");
		hidden.add_options()
			("help-color", "");

		boost::program_options::options_description cmdline_options;
		cmdline_options.add(generic).add(config).add(hidden);

		boost::program_options::options_description config_file_options;
		config_file_options.add(config);

		boost::program_options::options_description visible;
		visible.add(generic).add(config);

		boost::program_options::variables_map vm;

		std::ifstream ifs("GenConfig.cfg");
		store(parse_config_file(ifs, config_file_options), vm);
		notify(vm);
		
		if (vm.count("im-width"))
		{
			default_param[0] = vm["im-width"].as<int>();
		}
		
		if (vm.count("im-height"))
		{
			default_param[1] = vm["im-height"].as<int>();
		}

		if (vm.count("file"))
		{
			file = new char[vm["file"].as<std::string>().size() + 1]();
			strcpy(file, vm["im-height"].as<std::string>().c_str());
			file[vm["file"].as<std::string>().size()] = '\0';
		}
		
		if (vm.count("color"))
		{
			default_param[3] = vm["color"].as<int>();
		}
		
		if (vm.count("super-sampling"))
		{
			default_param[2] = vm["super-sampling"].as<int>();
		}

        if (vm.count("blocHeight"))
		{
			blocHeight = vm["blocHeight"].as<int>();
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
		if (vm2.count("im-width"))
		{
			default_param[0] = vm2["im-width"].as<int>();
		}
		
		if (vm2.count("im-height"))
		{
			default_param[1] = vm2["im-height"].as<int>();
		}

		if (vm2.count("file"))
		{
			std::string s = vm2["file"].as<std::string>();

			if(file)
				delete [] file;

			file = new char[s.size() + 1]();
			strcpy(file, s.c_str());
			file[s.size()] = '\0';
		}
		
		if (vm2.count("color"))
		{
			default_param[3] = vm2["color"].as<int>();
		}
		
		if (vm2.count("super-sampling"))
		{
			default_param[3] = vm2["super-sampling"].as<int>();
		}

        if (vm2.count("blocHeight"))
		{
			blocHeight = vm2["blocHeight"].as<int>();
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

		std::ofstream ofs("GenConfig.cfg",std::ofstream::trunc);
		ofs << "im-width=" << default_param[0] << std::endl
			<< "im-height=" << default_param[1] << std::endl
			<< "color=" << default_param[3] << std::endl
			<< "super-sampling=" << default_param[2] << std::endl
            << "blocHeight=" << blocHeight << std::endl;
 
		if (vm2.count("config") || default_param[1] % blocHeight != 0)
		{
			for(int i = 1; i < size; i++)
			MPI_Send(NULL, 0, MPI_INT, i, END, MPI_COMM_WORLD);
			return false;
		}
	}
	catch(std::exception& E)
	{
		std::cout << E.what() << std::endl;
	}

    int tmp = default_param[1];
    default_param[1] = blocHeight;

    std::string r;
    r = file;
    r.resize(r.find_last_of('/'));
    
    Mandelbrot::rep = new char[r.size()]();
    strcpy(Mandelbrot::rep, r.c_str());

    for(int i = 1; i < size; i++)
    {
        MPI_Send(Mandelbrot::rep, strlen(Mandelbrot::rep), MPI_CHAR, i, REP_SEND, MPI_COMM_WORLD);
    }

    MPI_Bcast(default_param, 4, MPI_INT, 0, MPI_COMM_WORLD);
    
    default_param[1] = tmp;
    return true;
}

bool receiveGenOptions()
{
	int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    MPI_Status status;
    int count;

    MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    if(status.MPI_TAG == END)
        return false;

    MPI_Get_count(&status, MPI_CHAR, &count);
    Mandelbrot::rep = new char[count + 1]();
    MPI_Recv(Mandelbrot::rep, count, MPI_CHAR, 0, REP_SEND, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    Mandelbrot::rep[count] = '\0';

    int default_param[4];
    MPI_Bcast(default_param, 4, MPI_INT, 0, MPI_COMM_WORLD);
    Mandelbrot::im_width = default_param[0];
    Mandelbrot::im_height = default_param[1];
    Mandelbrot::surEchantillonage = default_param[2];
    Mandelbrot::color = default_param[3];

    return true;
}

void decomposeWork(char* buf, mpf_t x, mpf_t y, mpf_t w, mpf_t h)
{
    int prec;
    char* tmp;

    tmp = strtok( buf, ":");

    tmp = strtok(NULL, ":");
    prec = ceil(strlen(tmp)*log(10)/log(2));
    prec = (prec%64 != 0)?(prec/64)*64+64:(prec/64)*64;
    prec = (prec < 64)?64:prec;

    mpf_init2( x, prec);
    mpf_set_str( x, tmp, 10);


    tmp = strtok(NULL, ":");
    prec = ceil(strlen(tmp)*log(10)/log(2));
    prec = (prec%64 != 0)?(prec/64)*64+64:(prec/64)*64;
    prec = (prec < 64)?64:prec;

    mpf_init2( y, prec);
    mpf_set_str( y, tmp, 10);


    tmp = strtok(NULL, ":");
    prec = ceil(strlen(tmp)*log(10)/log(2));
    prec = (prec%64 != 0)?(prec/64)*64+64:(prec/64)*64;
    prec = (prec < 64)?64:prec;

    mpf_init2( w, prec);
    mpf_set_str( w, tmp, 10);


    tmp = strtok(NULL, ":");
    prec = ceil(strlen(tmp)*log(10)/log(2));
    prec = (prec%64 != 0)?(prec/64)*64+64:(prec/64)*64;
    prec = (prec < 64)?64:prec;

    mpf_init2( h, prec);
    mpf_set_str( h, tmp, 10);
}

void getSubImages(std::queue<char*> *work, mpf_t x, mpf_t y, mpf_t w, mpf_t h, int imgHeight, int imWidth, int blocHeight)
{
    static int img_num = 0;

    std::vector<int> divs;
    divs.push_back(2);

    mpf_t nh,ny;
    mpf_init2(nh,mpf_get_prec(h));
    mpf_init2(ny,mpf_get_prec(y));

    int N = imgHeight / blocHeight;

    mpf_div_ui( nh, h, N); 
    mpf_sub( ny, h, nh);
    mpf_div_ui(ny,ny,2);

    mpf_add(ny,y,ny);

    std::stringstream info;
    std::stringstream header;
    header << "P6\n"<< imWidth << " " << imgHeight << "\n255\n";

    for (int i=0;i<N;i++)
    {
        char* buf = create_work( 0, x, ny, w, nh, divs);

        int start = (header.str().size() + 1) + imgHeight * imWidth * 3 - (i+1) * blocHeight * imWidth * 3;

        info << img_num << "|" << start << "|" << buf;

        delete buf;
        buf = new char[info.str().size() + 1]();

        strcpy(buf, info.str().c_str());
        buf[info.str().size()] = '\0';

        work->push(buf);

        mpf_sub(ny,ny,nh);
        info.str("");
    }

    img_num++;
}

void handler2(int argc, char** argv)
{
    init_top10();

    
    MPI_Status status;
    int size, count;
    char* buf;
    std::queue<int> *waiting = new std::queue<int>();
    std::queue<char*> *work = new std::queue<char*>();


    MPI_Comm_size(MPI_COMM_WORLD, &size);


    std::vector<int> divs;
    divs.push_back(2);
    divs.push_back(3);




    int default_param[4];
	//PARAMETRES PAR DEFAULT, A NE PAS CHANGER
	// largeur
	default_param[0] = 1920;
	// hauteur
	default_param[1] = 1080;
	// sur echantillonage
	default_param[2] = 4;
	// couleur
	default_param[3] = RAINBOW;
    // blocHeight
    int blocHeight = 54;
    

    bool verbose = false;
    char* file = NULL;

    if(!getGenOptions( argc, argv, default_param, file, verbose, size, blocHeight))
    	return;


    FILE* f = fopen(file, "r");
    if(f == NULL)
    {
    	perror("fopen");
    	MPI_Abort(MPI_COMM_WORLD, 911);
    }
    buf = new char[2049];

    mpf_t x, y, w, h;

    int nbr_img = 0;
    while(fgets(buf, 2048, f))
    {
        img_init( Mandelbrot::rep, nbr_img++, default_param[1], default_param[0]);

        //work->push(buf);
        //buf = new char[2049];
        decomposeWork(buf, x, y, w, h);
        getSubImages(work, x, y, w, h, default_param[1], default_param[0], blocHeight);
    }
    free(buf);
    mpf_clears( x, y, w, h, NULL);
    
    fclose(f);

    int img_count = 0, info[2];
    int flag;

    uint64_t tick = rdtsc();
    while(1)
    {
        while(1)
        {
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
            if(flag)
                break;
            else
                usleep(100);
        }

        if (status.MPI_TAG == INFO_RQST)
        {
            MPI_Recv(NULL, 0, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            info[0] = (int)work->size();
            info[1] = img_count;
            MPI_Send(info, 2, MPI_INT, status.MPI_SOURCE, INFO_RQST, MPI_COMM_WORLD);
            img_count++;
        } 
        else if (status.MPI_TAG == WORK_SEND) 
        {
            MPI_Get_count(&status, MPI_CHAR, &count);
            buf = new char[count+1]();
            MPI_Recv(buf, count, MPI_CHAR, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            buf[count] = '\0';
            // TODO meh ... tout sauf efficace .... a changer ...
            
            if(waiting->size() > 0)
            {

                MPI_Send(buf, count, MPI_CHAR, waiting->front(), WORK_SEND, MPI_COMM_WORLD);
                waiting->pop();
            }
            else
                work->push(buf);
        }
        else if (status.MPI_TAG == WORK_RQST)
        {
            MPI_Recv(NULL, 0, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if(work->size() > 0)
            {
                char* msg = work->front();
                MPI_Send(msg, strlen(msg), MPI_CHAR, status.MPI_SOURCE, WORK_SEND, MPI_COMM_WORLD);
                work->pop();
            }
            else
            {
                waiting->push(status.MPI_SOURCE);
                if(waiting->size() == (unsigned int)size - 1)
                {
                    for(int i = 1; i < size; i++)
                        MPI_Send(NULL, 0, MPI_INT, i, END, MPI_COMM_WORLD);

                    delete work;
                    delete waiting;

                    break;
                }
            }
        }
    }

    
    if(verbose)
    {
        std::cout <<"Time spend in cycle : "<< rdtsc() - tick << std::endl;
        FILE* f = fopen("log.txt", "a");
        fprintf(f, "%d %lu\n", size, rdtsc() - tick);
        fclose(f);
    }
}


void worker2(int argc, char** argv)
{
    init_top10();
    
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    int img_num = 0, start;
    
    MPI_Status status;
    int count;
    char* buf;

    if(!receiveGenOptions())
    	return;


    while(1)
    {
        MPI_Send(NULL, 0, MPI_INT, 0, WORK_RQST, MPI_COMM_WORLD);
        MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if(status.MPI_TAG == WORK_SEND)
        {
            MPI_Get_count(&status, MPI_CHAR, &count);
            buf = new char[count + 1]();
            
            MPI_Recv(buf, count, MPI_CHAR, 0, WORK_SEND, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            buf[count] = '\0';

            char* tmp = strtok(buf, "|");
            img_num = atoi(tmp);
            tmp = strtok(NULL, "|");
            start = atoi(tmp);

            tmp = strtok(NULL, "|");
            Mandelbrot* m = new Mandelbrot(tmp);

            delete [] buf;

            m->escapeSpeedCalcPar();
            m->save(img_num, start);

            delete m;
        }
        else if(status.MPI_TAG == END)
        {
            break;
        }
    }
}

