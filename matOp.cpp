#include "matOp.hpp"
#include <mpi.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

using namespace cv;
using namespace std;
#define dbg std::cout<<"line : "<<__LINE__<<", function : "<<__FUNCTION__<<"\n";

void img_init(char* rep, int img_num, int height, int width)
{
    stringstream nom_img("");
    nom_img << rep << "/mandel" << img_num << ".ppm";

    stringstream entete("");
    entete << "P6\n"<< width << " " <<height << "\n255\n";

    int file_section = open(nom_img.str().c_str(), O_CREAT | O_WRONLY, 0600);
    int ret = ftruncate(file_section, (height * width * 3 + entete.str().size()) * sizeof(char));
    if(ret == -1)
    {
        perror("ftruncate");
        std::cerr << errno << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 42);
    }   

    FILE *fd = fopen(nom_img.str().c_str(), "w");
    fprintf(fd, "%s", entete.str().c_str());
    fclose(fd);
}

void img_partial_save(int start, int width, int height, Mat* mat, char* rep, int img_num)
{
    stringstream nom_img("");
    nom_img << rep << "/mandel" << img_num << ".ppm";
    
    char* recopie = (char*)malloc(3*width*height*sizeof(char));

    for (int i = 0; i < height; i+=3)
    {
        for (int j = 0; j < width; j++)
        {
            Vec3b rgb = mat->at<char>(i, j);
            
            recopie[i*width+j] = rgb[2];
            recopie[i*width+j + 1] = rgb[1];
            recopie[i*width+j + 2] = rgb[0];
        }
    }

    FILE *fd = fopen(nom_img.str().c_str(), "w");
    fseek(fd, start, SEEK_SET);
    fwrite(recopie, sizeof(char), 3*width*height, fd);
    fclose(fd);
}