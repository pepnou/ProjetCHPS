#include "matOp.hpp"

using namespace cv;
using namespace std;

int matSave(Mat* mat, char* rep, double t)
{
	int tmp = t;
	
	static int num = 0;
	vector<int> compression_params;
    compression_params.push_back( IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(9);
	
	stringstream nom_img("");
	nom_img << "mkdir -p ../img/" << rep << "/" << tmp << "/";
	
	system(nom_img.str().c_str());
	
	nom_img.str("");
	nom_img << "../img/" << rep << "/" << tmp << "/mandel" << num++ << ".png";

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
