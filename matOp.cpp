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

void img_init(char* rep, int img_num, int height, int width)
{
    stringstream nom_img("");
    nom_img << rep << "/mandel" << img_num << ".png";

    stringstream entete("");
    entete << "P6\n"<< width << " " <<height << "\n256\n"; 

    int file_section = open(nom_img, O_CREAT, 0600);
    ftruncate(file_section, (height*width*3*sizeof(char)+strlen(entete)));
    FILE *fd = fopen(nom_img, "w");
    fprintf(fd, "%s", entete);
    fclose(fd);
}

void img_partial_save(int start, int width, int height, Mat* mat, char* rep, int img_num)
{
    stringstream nom_img("");
    nom_img << rep << "/mandel" << img_num << ".png";
    
    char recopie = malloc(3*width*height*sizeof(char));

    for (int i = 0; i < width*height; i+=3)
    {
        for (int j = 0; j < 3; ++)
        {
            recopie[i+j] = mat->at<char>(i, j);
        }
    }

    FILE *fd = fopen(nom_img, "w");
    fseek(fd, start, SEEK_SET);
    fwrite(recopie);
    fclose(fd);
}