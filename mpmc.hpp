#ifndef __MPMC__
#define __MPMC__

#include <cstddef>
#include <atomic>
#include <iostream>
#include <thread>

#include "mandelbrot.hpp"
class Mandelbrot;

#define offset_t size_t

typedef struct 
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
}threadDraw;

class Mpmc
{
	private:
		size_t size;
		work* buf;
		std::atomic<offset_t> last_write;
		std::atomic<offset_t> write_ok;
		std::atomic<offset_t> last_read;
		std::atomic<offset_t> read_ok;

	public:
		Mpmc(size_t size);
		~Mpmc();
		void push(work arg);
		bool pop(/*work* arg*/);
};

class MyThreads
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

void mainThread(void* arg);

/*std::thread* createThread(int nbT, Mpmc* mpmc);
void joinThread(int nbT, std::thread* threads, Mpmc* mpmc);
void mainThread(void* arg);
void terminate(void* arg);*/

#endif
