#include "matOp.hpp"

using namespace cv;
using namespace std;
#define dbg std::cout<<"line : "<<__LINE__<<", function : "<<__FUNCTION__<<"\n";

int matSave(Mat* mat, char* rep)
{	
	static int num = 0;
	vector<int> compression_params;
    compression_params.push_back( IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(9);
	
<<<<<<< HEAD
	stringstream nom_img("");
	nom_img << "mkdir -p ../img/" << rep << "/";
	
	system(nom_img.str().c_str());
	
	nom_img.str("");
	nom_img << "../img/" << rep << "/mandel" << num++ << ".png";
=======
	char nom_img[128];

	sprintf( nom_img, "mkdir -p ../img/%s", rep);
	system(nom_img);
>>>>>>> ANTOINE

    cout << nom_img.str() << endl;

	try
    {
        imwrite(nom_img.str().c_str(), *(mat), compression_params);
    }
    catch (const Exception& ex)
    {
        fprintf(stderr, "Exception converting image to PNG format: %s\n", ex.what());
    }
    return num-1;
}

int frameSave(Mat mat, char* rep, int num, int ligne)
{
	vector<int> compression_params;
    compression_params.push_back( IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(9);
	
	stringstream nom_img("");
	nom_img << "mkdir -p ../video/" << rep << "/frames/" << num << "/";
	
	system(nom_img.str().c_str());
	
	nom_img.str("");
	nom_img << "../video/" << rep << "/frames/" << num << "/" << ligne << ".png";

	//cout << nom_img.str() << endl;
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
