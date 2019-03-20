#include "matOp.hpp"
#include <mpi.h>

using namespace cv;
using namespace std;
#define dbg std::cout<<"line : "<<__LINE__<<", function : "<<__FUNCTION__<<"\n";

void matSave(Mat* mat, char* rep, int img_num)
{	
    vector<int> compression_params;
    compression_params.push_back( IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(9);
	
    //int rank;
    //MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    stringstream nom_img("");
    //nom_img << rep << "/mandel" << rank << "-" << num++ << ".png";
    nom_img << rep << "/mandel" << img_num << ".png";

    cout << nom_img.str() << endl;

    try
    {
        imwrite(nom_img.str().c_str(), *(mat), compression_params);
    }
    catch (const Exception& ex)
    {
        fprintf(stderr, "Exception converting image to PNG format: %s\n", ex.what());
    }
}

int frameSave(Mat mat, char* rep, int num, int ligne)
{
	vector<int> compression_params;
    compression_params.push_back( 16);
    compression_params.push_back(9);
	
	stringstream nom_img("");
	nom_img << "mkdir -p ../video/" << rep << "/frames/" << num << "/";
	
	system(nom_img.str().c_str());
	
	nom_img.str("");
	nom_img << "../video/" << rep << "/frames/" << num << "/" << ligne << ".png";

	try
    {
        imwrite(nom_img.str().c_str(), mat, compression_params);
    }
    catch (const Exception& ex)
    {
        fprintf(stderr, "Exception converting image to PNG format: %s\n", ex.what());
    }
    return num-1;
}
