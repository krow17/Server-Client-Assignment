#include "semaphore.H"
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

using namespace std;

Semaphore::Semaphore(int _val)
{
	value = _val;
	pthread_mutex_init(&m, NULL);
	pthread_cond_init(&c, NULL);
}

Semaphore::~Semaphore()
{
	pthread_mutex_destroy(&m);
	pthread_cond_destroy(&c);
}

int Semaphore::P()
{
	pthread_mutex_lock(&m);
	while (value <= 0)
		pthread_cond_wait(&c, &m);
	value--;
	pthread_mutex_unlock(&m);
}

int Semaphore::V()
{
	pthread_mutex_lock(&m);
	value++;
	pthread_mutex_unlock(&m);
	pthread_cond_signal(&c);
}
