#ifndef __MPMC__
#define __MPMC__

#include <cstddef>
#include <atomic>
#include <iostream>

#define offset_t size_t

typedef struct 
{
	void* arg;
	void (*f)(void*);
}work;

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
		void pop(work* arg);
};

#endif