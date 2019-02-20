#include <mpi.h>
#include <cstdlib>
#include <iostream>

#include "workHandler.hpp"


int main(int argc, char** argv)
{
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if(size < 2)
    {
        std::cerr << "Program must be called with at least 2 processes" << std::endl;
        std::exit(0);
    }

    std::cout << rank + 1 << " / " << size << std::endl;

    if(rank == 0)
        handler(argc, argv);
    else
        worker(argc, argv);

    MPI_Finalize();

    std::exit(0);
}
