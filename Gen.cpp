#include <mpi.h>
#include <cstdlib>
#include <iostream>

#include "workHandler.hpp"
#include "mandelbrot.hpp"

int Mandelbrot::surEchantillonage;
int Mandelbrot::im_width;
int Mandelbrot::im_height;
int Mandelbrot::color;
char* Mandelbrot::rep;
keyed_char* Mandelbrot::top10 = new keyed_char[11];
Mpmc* Mandelbrot::mpmc;

int main(int argc, char** argv)
{
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    std::cout << rank + 1 << " / " << size << std::endl;

    if(rank == 0)
        handler2(argc, argv);
    else
        worker2(argc, argv);

    MPI_Finalize();

    std::exit(0);
}
