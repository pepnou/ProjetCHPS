#include <mpi.h>
#include <cstdlib>

#include "workHandler.hpp"


int main(int argc, char** argv)
{
    MPI_Init(&argc, &argv);
    
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(rank == 0)
        handler(argc, argv);
    else
        worker(argc, argv);

    MPI_Finalize();

    std::exit(0);
}
