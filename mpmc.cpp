#include "mpmc.hpp"

#include <string.h>

#define state 0
#define last_write sizeof(char)+0*sizeof(size_t)
#define write_ok sizeof(char)+1*sizeof(size_t)
#define last_read sizeof(char)+2*sizeof(size_t)
#define read_ok sizeof(char)+3*sizeof(size_t)

Mpmc::Mpmc(size_t size) : size(size)
{
	//std::cout << __FUNCTION__ << " deb" << std::endl;

	MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

	//MPI_Win_allocate(size, 1, MPI_INFO_NULL, MPI_COMM_WORLD, &buf, &window);
	MPI_Alloc_mem(size, MPI_INFO_NULL, &buf);
	MPI_Win_create(buf, size, 1, MPI_INFO_NULL, MPI_COMM_WORLD, &window);

	memset( buf, 0, size);
	//std::cout << __FUNCTION__ << " fin" << std::endl;
}

Mpmc::~Mpmc()
{
	//std::cout << __FUNCTION__ << " deb" << std::endl;

	MPI_Win_free( &window );

	//std::cout << __FUNCTION__ << " fin" << std::endl;
}

char Mpmc::getState(size_t target)
{
	//std::cout << __FUNCTION__ << " deb" << std::endl;

	char result;

	MPI_Win_lock(MPI_LOCK_SHARED, target, 0, window);
	//MPI_Get(&result, 1, MPI_CHAR, target, state, 1, MPI_CHAR, window);
	MPI_Get_accumulate(NULL, 0, MPI_CHAR, &result, 1, MPI_CHAR, target, state, 1, MPI_CHAR, MPI_NO_OP, window);
	MPI_Win_unlock(target, window);

	//std::cout << __FUNCTION__ << " fin" << std::endl;
	
	return result;
}

void Mpmc::setState(char goal)
{
	//std::cout << __FUNCTION__ << " deb" << std::endl;

	MPI_Win_lock(MPI_LOCK_EXCLUSIVE, mpi_rank, 0, window);
	//MPI_Put(&goal, 1, MPI_CHAR, mpi_rank, state, 1, MPI_CHAR, window);
	MPI_Accumulate(&goal, 1, MPI_CHAR, mpi_rank, state, 1, MPI_CHAR, MPI_REPLACE, window);
	MPI_Win_unlock(mpi_rank, window);

	//std::cout << __FUNCTION__ << " fin" << std::endl;
}

int Mpmc::push(char* work)
{
	//std::cout << __FUNCTION__ << " deb" << std::endl;

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

	MPI_Win_lock(MPI_LOCK_EXCLUSIVE, mpi_rank, 0, window);

	size_t current, next, result;
	bool loop = true;

	size_t length = strlen(work) + 1;
	size_t msg_size = sizeof(size_t) + length;

	size_t header_size = sizeof(char) + 4 * sizeof(size_t);

	do 
	{
		MPI_Get(&current, 1, MPI_UNSIGNED_LONG, mpi_rank, last_write, 1, MPI_UNSIGNED_LONG, window);
		next = ((current + msg_size - header_size) % (size - header_size)) + header_size;
		
		if( (current < next && !( *((size_t*)buf + read_ok) > current && *((size_t*)buf + read_ok) < next ) ) 
			|| ( current > next && *((size_t*)buf + read_ok) <= current && *((size_t*)buf + read_ok) > next ) )
		{
			MPI_Compare_and_swap(&next, &current, &result, MPI_UNSIGNED_LONG, mpi_rank, last_write, window);
			if(result == next)
			{
				MPI_Put(&length, 1, MPI_UNSIGNED_LONG, mpi_rank, current, 1, MPI_UNSIGNED_LONG, window);
				MPI_Put(work, 1, MPI_UNSIGNED_LONG, mpi_rank, current + sizeof(size_t), 1, MPI_UNSIGNED_LONG, window);
				
				do
				{
					MPI_Compare_and_swap(&next, &current, &result, MPI_UNSIGNED_LONG, mpi_rank, write_ok, window);
				} while ( result != next);
				
				loop = false;
			}
		}
		else
		{
			MPI_Win_unlock(mpi_rank, window);
			//std::cout << __FUNCTION__ << " fin" << std::endl;
			return MPMC_FULL;
		}
	} while(loop);
	
	MPI_Win_unlock(mpi_rank, window);
	//std::cout << __FUNCTION__ << " fin" << std::endl;
	return MPMC_SUCCES;
}

int Mpmc::pop(size_t target, char* work)
{
	//std::cout << __FUNCTION__ << " deb" << std::endl;

	std::cerr << target << std::endl;
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

	size_t header_size = sizeof(char) + 4 * sizeof(size_t);

	size_t current, next, result, size, w_ok;
	bool loop = true;

	MPI_Win_lock(MPI_LOCK_EXCLUSIVE, target, 0, window);

	do
	{
		std::cerr << 0 << std::endl;
		MPI_Get(&current, 1, MPI_UNSIGNED_LONG, target, last_read, 1, MPI_UNSIGNED_LONG, window);
		std::cerr << 1 << std::endl;
		MPI_Get(&w_ok, 1, MPI_UNSIGNED_LONG, target, sizeof(size_t), 1, MPI_UNSIGNED_LONG, window);
		std::cerr << 2 << std::endl;

		if((w_ok - current + size) % size > 0)
		{
			std::cerr << 3 << std::endl;
			MPI_Get(&size, 1, MPI_UNSIGNED_LONG, target, current, 1, MPI_UNSIGNED_LONG, window);
			std::cerr << 4 << std::endl;
			next = (current - header_size + size + sizeof(size_t)) % (size - header_size) + header_size;

			if(size != 0)
			{
				MPI_Compare_and_swap(&next, &current, &result, MPI_UNSIGNED_LONG, target, last_read, window);
				if( result == next)
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
			}
		}
		else
		{
			MPI_Win_unlock(target, window);
			//std::cout << __FUNCTION__ << " fin" << std::endl;
			return MPMC_EMPTY;
		}
		
	} while(loop);

	MPI_Win_unlock(target, window);
	//std::cout << __FUNCTION__ << " fin" << std::endl;
	return MPMC_SUCCES;
}


int Mpmc::tryPop(size_t target)
{
	//std::cout << __FUNCTION__ << " deb" << std::endl;

	size_t current, size, w_ok;

	MPI_Win_lock(MPI_LOCK_EXCLUSIVE, target, 0, window);

	MPI_Get(&current, 1, MPI_UNSIGNED_LONG, target, last_read, 1, MPI_UNSIGNED_LONG, window);
	MPI_Get(&w_ok, 1, MPI_UNSIGNED_LONG, target, sizeof(size_t), 1, MPI_UNSIGNED_LONG, window);

	if((w_ok - current + size) % size > 0)
	{
		MPI_Get(&size, 1, MPI_UNSIGNED_LONG, target, current, 1, MPI_UNSIGNED_LONG, window);

		if(size != 0)
		{
			MPI_Win_unlock(target, window);
			//std::cout << __FUNCTION__ << " fin" << std::endl;
			return MPMC_SUCCES;
		}
	}

	MPI_Win_unlock(target, window);
	//std::cout << __FUNCTION__ << " fin" << std::endl;
	return MPMC_EMPTY;
}