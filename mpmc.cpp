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
		if((this->last_read.load() + 1 - tmp + this->size) % this->size > 0)
		{
			if(this->last_write.compare_exchange_strong( tmp, (tmp + 1) % this->size))
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

bool Mpmc::pop(/*work* arg*/)
{
	offset_t current, next;
	bool loop = true;
	do {
		current = this->last_read.load();
		next = (current + 1) % this->size;
		if((this->last_write.load() - current + this->size) % this->size > 0)
		{
			if(this->last_read.compare_exchange_strong( current, next))
			{
				while(this->write_ok.load() - next < 0);
				if(this->buf[next].f == exitThread)
					return false;
				this->buf[next].f(this->buf[next].arg);
				while(!this->read_ok.compare_exchange_strong( current, next));
				loop = false;
			}
		}
	} while(loop);
	return true;
}


MyThreads::MyThreads(int nbT) : nbT(nbT)
{
	this->threads = new thread[nbT];
	this->mpmc = new Mpmc(nbT*10);
	for(int i = 0; i < nbT; i++)
	{
		this->threads[i] = thread( mainThread, (void*)mpmc);
	}
}

MyThreads::~MyThreads()
{
	work w;
	w.f = exitThread;
	w.arg = nullptr;
	for(int i = 0; i < this->nbT; i++)
	{
		this->mpmc->push(w);
	}
	
	for(int i = 0; i < this->nbT; i++)
	{
		this->threads[i].join();
	}

	delete [] this->threads;
	delete this->mpmc;
}

void mainThread(void* arg)
{
	Mpmc* mpmc = (Mpmc*)arg;
	while(mpmc->pop());
}

void exitThread(void* arg)
{}

Mpmc* MyThreads::getMpmc()
{
	return this->mpmc;
}
