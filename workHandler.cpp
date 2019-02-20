#include "workHandler.hpp"
#include <mpi.h>
#include <queue>
#include <string.h>

#define SIZE_RQST 0
#define WORK_SEND 1
#define WORK_RQST 2
#define END 3


void handler(int argc, char** argv)
{
    MPI_Status status;
    int size, count;
    char* buf;
    std::queue<int> *waiting = new std::queue<int>();
    std::queue<char*> *work = new std::queue<char*>();


    MPI_Comm_size(MPI_COMM_WORLD, &size);

    while(1)
    {
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if (status.MPI_TAG == SIZE_RQST)
        {
            int tmp = (int)work->size();
            MPI_Send(&(tmp), 1, MPI_INT, status.MPI_SOURCE, SIZE_RQST, MPI_COMM_WORLD);
        } 
        else if (status.MPI_TAG == WORK_SEND) 
        {
            MPI_Get_count(&status, MPI_CHAR, &count);
            buf = new char[count]();
            //TODO meh ... tout sauf efficace .... a changer ...
            
            if(waiting->size() > 0)
            {
                MPI_Send(buf, count, MPI_CHAR, waiting->front(), WORK_SEND, MPI_COMM_WORLD);
                waiting->pop();
            }
            else
                work->push(buf);
        }
        else if (status.MPI_TAG == WORK_RQST)
        {
            if(work->size() > 0)
            {
                char* msg = work->front();
                MPI_Send(msg, strlen(msg), MPI_CHAR, status.MPI_SOURCE, WORK_SEND, MPI_COMM_WORLD);
                work->pop();
            }
            else
            {
                waiting->push(status.MPI_SOURCE);
                if(waiting->size() == size - 1)
                {
                    for(int i = 0; i < size; i++)
                        MPI_Send(NULL, 0, MPI_INT, i, END, MPI_COMM_WORLD);

                    delete work;
                    delete waiting;

                    break;
                }
            }
        }
    }
}

void worker(int argc, char** argv)
{
    MPI_Status status;
    int count;
    char* buf;

    while(1)
    {
        MPI_Send(NULL, 0, MPI_INT, 0, WORK_RQST, MPI_COMM_WORLD);
        MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if(status.MPI_TAG == WORK_SEND)
        {
            MPI_Get_count(&status, MPI_CHAR, &count);
            buf = new char[count]();
            
            MPI_Recv(buf, count, MPI_CHAR, 0, WORK_SEND, MPI_COMM_WORLD, &status);

            // TODO traitement
        }
        else if(status.MPI_TAG == END)
        {
            break;
        }
    }
}
