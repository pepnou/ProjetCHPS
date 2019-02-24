#include <mpi.h>
#include <queue>
#include <string>
#include <string.h>
#include <boost/program_options.hpp>
#include <fstream>
#include <cmath>
#include <ctime>
#include <vector>

#include "mandelbrot.hpp"
#include "workHandler.hpp"
#include "rdtsc.hpp"

// namespace po = boost::program_options;

#define RAINBOW 1
#define JAUNE_BLEU 2
#define SINCOS 3

#define INFO_RQST 0
#define WORK_SEND 1
#define WORK_RQST 2
#define END 3
#define REP_SEND 4



int Mandelbrot::surEchantillonage;
int Mandelbrot::im_width;
int Mandelbrot::im_height;
int Mandelbrot::color;
char* Mandelbrot::rep;



void handler(int argc, char** argv)
{
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
    


    int enough = 1;
    bool verbose = false;

    mpf_t x, y, w, h;
    mpf_inits( x, y, w, h, NULL);

    mpf_set_d( x, -0.5);
    mpf_set_d( y, 0.0);
    mpf_set_d( w, 3);
    mpf_set_d( h, 2);

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

		std::ifstream ifs("Config.cfg");
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
		std::ofstream ofs("Config.cfg",std::ofstream::trunc);
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
                    return;
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
    {
        MPI_Send(Mandelbrot::rep, strlen(Mandelbrot::rep), MPI_CHAR, i, REP_SEND, MPI_COMM_WORLD);
        //std::cerr << "msg " << i << " " << REP_SEND << std::endl;
    }

    MPI_Bcast(default_param, 4, MPI_INT, 0, MPI_COMM_WORLD);
    
    std::stringstream cmd("");
    cmd << "mkdir -p " << r.str().c_str();
    system(cmd.str().c_str());

    buf = create_work(enough, x, y, w, h, divs);
    work->push(buf);

    int img_count = 0, info[2];


    uint64_t tick = rdtsc();
    while(1)
    {
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        //std::cerr << "recv " << status.MPI_SOURCE << " " << status.MPI_TAG << std::endl;

        if (status.MPI_TAG == INFO_RQST)
        {
            MPI_Recv(NULL, 0, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            //std::cerr << "size send" << std::endl;
            info[0] = (int)work->size();
            info[1] = img_count;
            MPI_Send(info, 2, MPI_INT, status.MPI_SOURCE, INFO_RQST, MPI_COMM_WORLD);
            img_count++;
            //std::cerr << "msg " << status.MPI_SOURCE << " " << SIZE_RQST << std::endl;
        } 
        else if (status.MPI_TAG == WORK_SEND) 
        {
            //std::cerr << "work received" << std::endl;
            MPI_Get_count(&status, MPI_CHAR, &count);
            buf = new char[count+1]();
            MPI_Recv(buf, count, MPI_CHAR, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            buf[count] = '\0';
            // TODO meh ... tout sauf efficace .... a changer ...
            
            if(waiting->size() > 0)
            {

                MPI_Send(buf, count, MPI_CHAR, waiting->front(), WORK_SEND, MPI_COMM_WORLD);
                //std::cerr << "msg " << waiting->front() << " " << WORK_SEND << std::endl;
                waiting->pop();
            }
            else
                work->push(buf);
        }
        else if (status.MPI_TAG == WORK_RQST)
        {
            MPI_Recv(NULL, 0, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            //std::cerr << "work request" << std::endl;
            if(work->size() > 0)
            {
                //std::cerr << "work send" << std::endl;
                char* msg = work->front();
                MPI_Send(msg, strlen(msg), MPI_CHAR, status.MPI_SOURCE, WORK_SEND, MPI_COMM_WORLD);
                //std::cerr << "msg " << status.MPI_SOURCE << " " << WORK_SEND << std::endl;
                work->pop();
            }
            else
            {
                waiting->push(status.MPI_SOURCE);
                if(waiting->size() == size - 1)
                {
                    //std::cerr << "exiting" << std::endl;
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

void worker(int argc, char** argv)
{
    //PARAMETRES PAR DEFAULT, A NE PAS CHANGER
    Mandelbrot::im_width = 1920;
    Mandelbrot::im_height = 1080;
    Mandelbrot::surEchantillonage = 4;
    Mandelbrot::color = RAINBOW;

   
    
    MPI_Status status;
    int count;
    char* buf;

    MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    //std::cerr << "recv " << status.MPI_SOURCE << " " << status.MPI_TAG << std::endl;
    if(status.MPI_TAG == END)
        return;
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


    while(1)
    {
        MPI_Send(NULL, 0, MPI_INT, 0, WORK_RQST, MPI_COMM_WORLD);
        //std::cerr << "msg " << 0 << " " << WORK_RQST << std::endl;
        MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        //std::cerr << "recv " << status.MPI_SOURCE << " " << status.MPI_TAG << std::endl;

        if(status.MPI_TAG == WORK_SEND)
        {
            //std::cerr << "processing work" << std::endl;

            MPI_Get_count(&status, MPI_CHAR, &count);
            buf = new char[count + 1]();
            
            MPI_Recv(buf, count, MPI_CHAR, 0, WORK_SEND, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            buf[count] = '\0';

            Mandelbrot* m = new Mandelbrot(buf);

            delete [] buf;

            m->dichotomie3();

            delete m;

            //std::cerr << "processing done" << std::endl;
        }
        else if(status.MPI_TAG == END)
        {
            //std::cerr << "exiting" << std::endl;
            return;
        }
    }
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

    for(int i = 0; i < divs.size(); i++)
        r << ":" << divs.at(i);

    char* res = new char[r.str().size() + 1]();
    strcpy(res, r.str().c_str());
    res[r.str().size()] = '\0';
    
    //std::cerr << res << std::endl;

    return res;
}

void getHandlerInfo(bool& needwork, int& img_num)
{
    MPI_Send(NULL, 0, MPI_INT, 0, INFO_RQST, MPI_COMM_WORLD);
    //std::cerr << "msg " << 0 << " " << SIZE_RQST << std::endl;
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
    //std::cerr << "msg " << 0 << " " << WORK_SEND << std::endl;
}
