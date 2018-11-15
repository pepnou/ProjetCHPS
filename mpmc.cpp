#include "mpmc.hpp"



Mpmc::Mpmc(size_t size) : size(size), buf(new work[size])
{}

Mpmc::~Mpmc()
{
	delete this->buf;
}

void Mpmc::push(work arg)
{

}

void Mpmc::pop(work* arg)
{
	int old_val;
	int tmp = this->last_read;
	//if((this->last_write - tmp + this->size) % this->size > 0)
		//compare_and_swap();
}
