#include "mpmc.hpp"

using namespace std;

Mpmc::Mpmc(size_t size) : size(size), buf(new work[size])
{
	this->last_write.store(0);
	this->write_ok.store(0);
	this->last_read.store(0);
	this->read_ok.store(0);
}

Mpmc::~Mpmc()
{
	delete this->buf;
}

void Mpmc::push(work arg)
{
	offset_t tmp;
	bool loop = true;
	do {
		tmp = this->last_write.load();
		if((this->last_read.load() - tmp + this->size) % this->size >= 0)
		{
			if(this->last_write.compare_exchange_strong( tmp, tmp+1))
			{
				while(this->read_ok.load() - ((tmp + 1) % this->size) < 0);
				this->buf[(tmp + 1) % this->size].arg = arg.arg;
				this->buf[(tmp + 1) % this->size].f = arg.f;
				while(!this->write_ok.compare_exchange_strong( tmp, (tmp + 1) % this->size));
				loop = false;
			}
		}
	} while(loop);
}

void Mpmc::pop(work* arg)
{
	offset_t tmp;
	bool loop = true;
	do {
		tmp = this->last_read.load();
		if((this->last_write.load() - tmp + this->size) % this->size > 0)
		{
			if(this->last_read.compare_exchange_strong( tmp, tmp+1))
			{
				while(this->write_ok.load() - ((tmp + 1) % this->size) < 0);
				arg->arg = this->buf[(tmp + 1) % this->size].arg;
				arg->f = this->buf[(tmp + 1) % this->size].f;
				while(!this->read_ok.compare_exchange_strong( tmp, (tmp + 1) % this->size));
				loop = false;
			}
		}
	} while(loop);
}
