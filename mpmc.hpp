#ifndef __MPMC__
#define __MPMC__

#include <cstddef>
#include <atomic>

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
		offset_t last_write = 0, write_ok = 0;
		offset_t last_read = 0, read_ok = 0;

	public:
		Mpmc(size_t size);
		~Mpmc();
		void push(work arg);
		void pop(work* arg);
};

#endif