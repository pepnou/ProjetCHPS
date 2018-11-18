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
	cout<<"id : "<<getpid()<<" push : ro "<<this->read_ok.load()<<" r "<<this->last_read.load()<<" wo "<<this->write_ok.load()<<" w "<<this->last_write.load()<<endl;
	offset_t current, next;
	bool loop = true;
	do {
		current = this->last_write.load();
		next = (current + 1) % this->size;
		
		//if((this->last_read.load() + 1 - current + this->size) % this->size > 0)
		if((this->read_ok.load() + 1 - current + this->size) % this->size > 0)
		//if(this->read_ok.load() != next)
		{
			if(this->last_write.compare_exchange_strong( current, next))
			{
				while(this->read_ok.load() - next < 0);
				this->buf[next].arg = arg.arg;
				this->buf[next].f = arg.f;
				while(!this->write_ok.compare_exchange_strong( current, next));
				loop = false;
			}
		}
	} while(loop);
}

bool Mpmc::pop(/*work* arg*/)
{
	cout<<"id : "<<getpid()<<" pop : ro "<<this->read_ok.load()<<" r "<<this->last_read.load()<<" wo "<<this->write_ok.load()<<" w "<<this->last_write.load()<<endl;
	offset_t current, next;
	bool loop = true;
	do {
		current = this->last_read.load();
		next = (current + 1) % this->size;
		
		//if((this->last_write.load() - current + this->size) % this->size > 0)
		if((this->write_ok.load() - current + this->size) % this->size > 0)
		//if(this->write_ok.load() != current)
		{
			if(this->last_read.compare_exchange_strong( current, next))
			{
				while(this->write_ok.load() - next < 0);
				if(this->buf[next].f == exitThread)
				{
					cout<<"eT"<<endl;
					return false;
				}
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
	dbg
	work w;
	w.f = exitThread;
	w.arg = nullptr;
	//cout<<this->nbT<<endl;
	for(int i = 0; i < this->nbT; i++)
	{
		cout<<i<<endl;
		this->mpmc->push(w);
		dbg
	}
	dbg
	for(int i = 0; i < this->nbT; i++)
	{
		this->threads[i].join();
		cout<<i<<" join"<<endl;
	}

	delete [] this->threads;
	delete this->mpmc;
}

void mainThread(void* arg)
{
	cout<<getpid()<<endl;
	dbg
	Mpmc* mpmc = (Mpmc*)arg;
	while(mpmc->pop());
	cout<<"fT"<<endl;
}

void exitThread(void* arg)
{}

Mpmc* MyThreads::getMpmc()
{
	return this->mpmc;
}
