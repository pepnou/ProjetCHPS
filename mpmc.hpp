#ifndef __MPMC__
#define __MPMC__

#define SUCCES 0
#define FULL 1
#define EMPTY 2

#include <cstddef>
//#include <atomic>
#include <iostream>
//#include <thread>
#include <mpi.h>

#include "mandelbrot.hpp"
class Mandelbrot;


/*typedef struct 
{
	void* arg;
	void (*f)(void*);
}work;

typedef struct 
{
	mpf_t *x;
	mpf_t *y;
	int deb, fin;
	Mandelbrot* M;
}threadDraw;*/

class Mpmc
{
	private:
		int mpi_rank, mpi_size;
		size_t size;
		MPI_Info info;
		MPI_Win window;
		char* buf;
		// buf : last_write;
		// buf + sizeof(size_t) : write_ok;
		// buf + 2*sizeof(size_t) : last_read;
		// buf + 3*sizeof(size_t) : read_ok;

	public:
		Mpmc(size_t size);
		~Mpmc();
		int push(char* work);
		int pop(size_t target, char* work);
};

/*class MyThreads
{
	private:
		std::thread* threads;
		int nbT;
		Mpmc* mpmc;

	public:
		MyThreads(int nbT);
		~MyThreads();
		Mpmc* getMpmc();
};

void mainThread(void* arg);*/

#endif
