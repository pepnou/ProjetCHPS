#include "matOp.hpp"

using namespace cv;
using namespace std;

int matSave(Mat* mat, char* rep)
{
	static int num = 0;
	vector<int> compression_params;
    compression_params.push_back( IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(9);
	
	char nom_img[128];
	sprintf( nom_img, "mkdir -p ../img/%s", rep);
	system(nom_img);

    sprintf( nom_img, "../img/%s/mandel%d.png", rep, num++);

	cout<<nom_img<<endl;
	try
    {
        imwrite(nom_img, *(mat), compression_params);
    }
    catch (const Exception& ex)
    {
        fprintf(stderr, "Exception converting image to PNG format: %s\n", ex.what());
    }
    return num-1;
}