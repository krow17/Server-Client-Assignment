#ifndef _BBuffer_H_
#define _BBuffer_H_

#include "semaphore.H"
#include <queue> //used vector initially, but didn't want to deal with compiling cpp and c files together
#include <pthread.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>


using namespace std;


class Person
{
	public:
		int ident;
		string response;


		Person(int i, string re)
		{
			ident = i;
			response = re;
		}

};
class BBuffer
{
	private:
		Semaphore * mutex;
		Semaphore * full;
		Semaphore * empty;
		queue<Person> buffer;

	public:
		int size;

		BBuffer(int _size)
		{
			size = _size;
			mutex = new Semaphore(1);
			full = new Semaphore(0);
			empty = new Semaphore(size);
		}

		~BBuffer()
		{
			delete mutex;
			delete full;
			delete empty;
		}

		void Deposit(Person item)
		{
			empty->P();
			mutex->P();
			buffer.push(item);
			mutex->V();
			full->V();
		}

		Person Remove()
		{
			full->P();
			mutex->P();
			Person item = buffer.front();
			buffer.pop();
			mutex->V();
			empty->V();
			return item;
		}
};

#endif
