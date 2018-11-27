#include "main.hpp"

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
	mpf_t x, y, w, h;
	const int NBR_THREADS = std::thread::hardware_concurrency();
	MyThreads* MT = new MyThreads(NBR_THREADS);
	//MyThreads* MT = new MyThreads(1);
	Mpmc* mpmc = MT->getMpmc();

	//C'EST ICI QUE TU CHANGES LES PARAMETRES POUR CHANGER LE RESULTAT FINAL BONHOMME !
	int im_w = 1920, im_h = 1080, surech = 4, iteration = 300, enough = 1, color = 1;
	//algo couleur :
	// 1 -> normal arc en ciel
	// 2 -> bleu blanc jaune noir
 	// 3 -> arc en ciel forme kirby

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

	
	delete MT;
	mpf_clears( x, y, w, h, NULL);	
	exit(0);
}
