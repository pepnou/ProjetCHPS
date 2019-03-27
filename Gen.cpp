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

    MPI_Info info;
    MPI_Comm node;
    MPI_Comm_split_type(MPI_COMM_WORLD, MPI_COMM_TYPE_SHARED, rank, info, &node);

    std::cout << rank + 1 << " / " << size << std::endl;

    if(rank == 0)
        handler2(argc, argv);
    else
        worker2(argc, argv);

    MPI_Finalize();

    std::exit(0);
}
