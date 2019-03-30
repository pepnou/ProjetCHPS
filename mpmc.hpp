#ifndef __MPMC__
#define __MPMC__

#define SUCCES 0
#define FULL 1
#define EMPTY 2

#include <cstddef>
#include <iostream>
#include <mpi.h>

class Mpmc
{
	private:
		int mpi_rank, mpi_size;
		size_t size;
		MPI_Info info;
		MPI_Win window;
		char* buf;

	public:
		Mpmc(size_t size);
		~Mpmc();
		int push(char* work);
		int pop(size_t target, char* work);

};

#endif
