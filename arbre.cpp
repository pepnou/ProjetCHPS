#include <iostream>
#include <sstream>
#include <mpi.h>
#include <cmath>

//using namespaces std; 

typedef struct Noeud
{
	int cle;
	struct Noeud* droite;
	struct Noeud* gauche;
    struct Noeud* pere;
} Noeud_t; 
typedef Noeud_t* arbre_t;

//Noeud* arbre = NULL;

void creer_fils(arbre_t& pos, int& cle, int currentHeigth, arbre_t& racine, int size, int height)
	{
		currentHeigth++;

		if(currentHeigth < height)
		{
			if(cle < size -1)
			{
				arbre_t fg = new Noeud_t();
				fg->gauche = NULL;
				fg->droite = NULL;
				fg->pere = pos;
				pos->gauche = fg;
				creer_fils(fg, cle, currentHeigth, racine, size, height);
			}

			pos->cle = ++cle;

			if(cle < size -1)
			{
				arbre_t fd = new Noeud_t();
				fd->gauche = NULL;
				fd->droite = NULL;
				fd->pere = pos;
				pos->droite = fd;
				creer_fils(fd, cle, currentHeigth, racine, size, height);
			}
		}
		else
			pos->cle = ++cle;
	}

	void creer_pere(arbre_t& pos, int& cle, int currentHeigth, arbre_t& racine, int size, int height)
	{
		racine = pos;

		currentHeigth--;
		pos->cle = ++cle;

		if(currentHeigth < height && cle < size - 1)
		{
			arbre_t fd = new Noeud_t();
			fd->gauche = NULL;
			fd->droite = NULL;
			fd->pere = pos;
			pos->droite = fd;
			creer_fils(fd, cle, currentHeigth, racine, size, height);
		}

		if(currentHeigth > 0)
		{
			arbre_t pere = new Noeud_t();
			pere->gauche = pos;
			pere->droite = NULL;
			pere->pere = NULL;
			pos->pere = pere;
			creer_pere(pere, cle, currentHeigth, racine, size, height);
		}
	}

	

	arbre_t cree_arbre()

	{ 
		int size;
		MPI_Comm_size(MPI_COMM_WORLD, &size);

		int height = log(size)/log(2);
		int currentHeigth = height + 1;
		int cle = -1;

		arbre_t arbre = new Noeud_t();
		arbre->gauche = NULL;
		arbre->droite = NULL;
		arbre->pere = NULL;

		arbre_t racine = arbre;

		creer_pere(arbre, cle, currentHeigth, racine, size, height);

		return racine;
	}

	int* findNode( arbre_t arbre, int rank)
	{
		if(arbre->cle == rank)
		{
			int* res = new int[3]();

			int nb_voisins = 0;
			if(arbre->pere)
			{
				res[nb_voisins] = arbre->pere->cle;
				nb_voisins++;
			}
			if(arbre->gauche)
			{
				res[nb_voisins] = arbre->gauche->cle;
				nb_voisins++;
			}
			if(arbre->droite)
			{
				res[nb_voisins] = arbre->droite->cle;
				nb_voisins++;
			}
			if(nb_voisins < 3)
				res[nb_voisins] = -1;
			return res;
		}
		else
		{
			if(rank < arbre->cle)
			{
				return findNode(arbre->gauche, rank);
			}
			else
			{
				return findNode(arbre->droite, rank);
			}
		}
	}

void supprArbre(arbre_t arbre)
{
	if(arbre->gauche)
		supprArbre(arbre->gauche);
	if(arbre->droite)
		supprArbre(arbre->droite);
	delete arbre;
}

int main(int argc, char** argv)
{
	MPI_Init(&argc, &argv);

	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	arbre_t a = cree_arbre();
	int* vois = findNode(a, rank);

	std::stringstream voisins("");
	voisins << rank << " : ";

	for(int i = 0; i < 3; i++)
	{
		if(vois[i] >= 0)
			voisins << vois[i] << " ";
		else
			break;
	}

	std::cout << voisins.str() << std::endl;

	supprArbre(a);


	MPI_Finalize();
	exit(0);
	
}