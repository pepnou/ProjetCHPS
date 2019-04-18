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

    int file_section = open(nom_img.str().c_str(), O_CREAT | O_RDWR, 0600);
    int ret = ftruncate(file_section, (height * width * 3 + entete.str().size()) * sizeof(char));
    if(ret == -1)
    {
        perror("ftruncate");
        std::cerr << errno << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 42);
    }   

    FILE *fd = fopen(nom_img.str().c_str(), "r+");
    fprintf(fd, "%s", entete.str().c_str());
    fclose(fd);
}

void img_partial_save(int start, int width, int height, Mat* mat, char* rep, int img_num)
{
    stringstream nom_img("");
    nom_img << rep << "/mandel" << img_num << ".ppm";
    
    unsigned char* recopie = (unsigned char*)malloc(3*width*height*sizeof(char));

    /*
    for (int i = 0; i < width*height; i+=3)
    {
        for (int i = 0; i < height; i++)
        {
            Vec3b rgb = mat->at<Vec3b>(i, j);
            
            recopie[i * width + j    ] = (unsigned char)rgb[2];
            recopie[i * width + j + 1] = (unsigned char)rgb[1];
            recopie[i * width + j + 2] = (unsigned char)rgb[0];

            std::cerr << (int)recopie[i * width + j] << " " << (int)recopie[i * width + j + 1] << " " << (int)recopie[i * width + j + 2] << std::endl;
        }
    }
    */

    for (int i = 0; i < height; ++i)
    {
        for (int j = 0; j < width; ++j)
        {
           recopie[(i*width + j)*3] = mat->at<Vec3b>(i, j)[1];
           recopie[(i*width + j)*3 + 1] = mat->at<Vec3b>(i, j)[0];
           recopie[(i*width + j)*3 + 2] = mat->at<Vec3b>(i, j)[2];
        }
    }

    FILE *fd = fopen(nom_img.str().c_str(), "r+");
    int ret = fseek(fd, start, SEEK_SET);
    if(ret == -1)
    {
        perror("fseek");
        MPI_Abort(MPI_COMM_WORLD, 44);
    }

    fwrite(recopie, sizeof(char), 3*width*height, fd);
    fclose(fd);
}
