#include "workHandler.hpp"
#include <mpi.h>

WorkHandler::WorkHandler()
{
    MPI_Comm_size(&size);
    waiting = new queue<int>();
    work = new queue<Work>();
}

WorkHandler::~WorkHandler()
{
    delete waiting;
    delete work;
}
