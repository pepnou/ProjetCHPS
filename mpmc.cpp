#include "mpmc.hpp"

//buf : last_write;
// buf + sizeof(size_t) : write_ok;
// buf + 2*sizeof(size_t) : last_read;
// buf + 3*sizeof(size_t) : read_ok;

// #define last_write (*((size_t*)buf))
// #define write_ok (*((size_t*)buf + sizeof(size_t)))
// #define last_read (*((size_t*)buf + 2 * sizeof(size_t)))
// #define read_ok (*((size_t*)buf + 3 * sizeof(size_t)))

#define last_write 0
#define write_ok sizeof(size_t)
#define last_read 2*sizeof(size_t)
#define read_ok 3*sizeof(size_t)

Mpmc::Mpmc(size_t size) : size(size)
{
	MPI_Win_allocate( size, 0, info, MPI_COMM_WORLD, buf, &window);
	MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
	
	memset( buf, 0, size);
}

Mpmc::~Mpmc()
{
	MPI_Win_free( &window );
}

int Mpmc::push(char* work)
{
	/*size_t current, next;
	bool loop = true;
	do {
		current = this->last_write.load();
		next = (current + 1) % this->size;
		
		if((this->read_ok.load() - current - 1 + this->size) % this->size > 0)
		{
			if(this->last_write.compare_exchange_strong( current, next))
			{
				this->buf[next] = arg;
				while(!this->write_ok.compare_exchange_strong( current, next));
				loop = false;
			}
		}
	} while(loop);*/

	size_t current, next, result;
	bool loop = true;

	size_t length = strlen(work) + 1;
	size_t msg_size = sizeof(size_t) + length;

	do 
	{
		current = *((size_t*)buf + last_write);
		next = (current + msg_size) % this->size;
		
		if( (current < next && !( *((size_t*)buf + read_ok) > current && *((size_t*)buf + read_ok) < next ) ) || ( current > next && *((size_t*)buf + read_ok) < current && *((size_t*)buf + read_ok) > next ) )
		{
			MPI_Compare_and_swap(&next, &current, &result, MPI_UNSIGNED_LONG, mpi_rank, last_write, window);
			if(result == next)
			{
				memcpy(&buf[current], &length, sizeof(size_t));
				memcpy(&buf[current + sizeof(size_t)], work, length);
				
				do
				{
					MPI_Compare_and_swap(&next, &current, &result, MPI_UNSIGNED_LONG, mpi_rank, write_ok, window);
				} while ( result != next);
				
				loop = false;
			}
		}
		else
		{
			return FULL;
		}
	} while(loop);
	
	return SUCCES;
}

int Mpmc::pop(size_t target, char* work)
{
	if(work)
		delete [] work;
	
	/*offset_t current, next;
	work w;
	bool loop = true;
	do {
		current = this->last_read.load();
		next = (current + 1) % this->size;
		
		if((this->write_ok.load() - current + this->size) % this->size > 0)
		{
			if(this->last_read.compare_exchange_strong( current, next))
			{
				if(this->buf[next].f == nullptr)
					return false;

				w = this->buf[next];
				while(!this->read_ok.compare_exchange_strong( current, next));
				w.f(w.arg);
				loop = false;
			}
		}
	} while(loop);
	return true;*/

	size_t current, next, result, size, w_ok, l_r, r_ok;
	bool loop = true;

	do {
		MPI_Get(&current, 1, MPI_UNSIGNED_LONG, target, 2 * sizeof(size_t), 1, MPI_UNSIGNED_LONG, window);
		MPI_Get(&size, 1, MPI_UNSIGNED_LONG, target, current, 1, MPI_UNSIGNED_LONG, window);
		next = (current + size + sizeof(size_t)) % size;
		
		/*MPI_Get(&w_ok, 1, MPI_UNSIGNED_LONG, target, sizeof(size_t), 1, MPI_UNSIGNED_LONG, window);

		if( (current < next && !( w_ok > current && w_ok < next ) ) || ( current > next && w_ok < current && w_ok > next ) )
		{*/
			MPI_Compare_and_swap(&next, &current, &result, MPI_UNSIGNED_LONG, target, last_read, window);
			if( result == next);
			{
				work = new char[size];
				MPI_Get(work, size, MPI_CHAR, target, current + sizeof(size_t), size, MPI_CHAR, window);

				char* empty = (char*)calloc(size+sizeof(size_t), sizeof(char));
				MPI_Put(empty, size + sizeof(size_t), MPI_CHAR, target, current, size + sizeof(size_t), MPI_CHAR, window);
				free(empty);

				do
				{
					MPI_Compare_and_swap(&next, &current, &result, MPI_UNSIGNED_LONG, target, read_ok, window);
				} while (result != next);
				
				loop = false;
			}
		/*}
		else
		{
			return EMPTY
		}*/
		
	} while(loop);
	return true;
}
