#include "load.hpp"

using namespace std;

void loading(int p)
{
	struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);

	int terminal_width = w.ws_col - 2;
	static int total, act = -1;
	static bool reset = true;
	int nbr;

	if(reset)
	{
		total = p;
		reset = false;
	}
	else if(p<0)
		reset = true;
	else
	{
		nbr = p * terminal_width / total;
		if(nbr != act)
		{
			system("clear");
			cout<<"[";
			for(int i = 0; i < nbr; i++)
				cout<<"#";
			for(int i = 0; i < terminal_width - nbr; i++)
				cout<<".";
			cout<<"]"<<endl;
			act = nbr;
		}
	}
}
