#include "load.hpp"

using namespace std;

#define STD "\033[0m"

#define BLA "\033[1;30m"
#define RED "\033[1;31m"
#define GRE "\033[1;32m"
#define YEL "\033[1;33m"
#define BLU "\033[1;34m"
#define MAG "\033[1;35m"
#define CYA "\033[1;36m"
#define WHI "\033[1;37m"


void loading(int p)
{
	struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
	int terminal_width = w.ws_col - 2, nbr;
	
	static int total, act = -1;
	static bool reset = true;

	if(reset)
	{
		cout<<p<<endl;
		total = p;
		reset = false;
	}
	else
	{
		if(p<0)
		{
			reset = true;
		}
		else
		{
			nbr = (float)p / total * terminal_width;
			
			if(nbr != act)
			{
				system("clear");
				cout<<GRE "Chargement :" STD<<endl<<YEL "[" STD;
				for(int i = 0; i < nbr; i++)
					cout<<GRE "#" STD;
				for(int i = 0; i < terminal_width - nbr; i++)
					cout<<RED "." STD;
				cout<<YEL "]" STD <<endl;
				act = nbr;
			}
		}
	}
}
