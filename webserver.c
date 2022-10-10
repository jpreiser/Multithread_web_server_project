#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "webserver.h"

#define MAX_REQUEST 10

// the buffer to be shared among the threads
int request[MAX_REQUEST];

int port;
int in;
int out;

// semaphores and lock
sem_t sem_full;
sem_t sem_empty;
pthread_mutex_t mutex;

void single_threaded_server(int sock)
{
	int s;
	while (1)
	{
		s = accept(sock, NULL, NULL);
		if (s < 0) break;
		process(s);
	}
}

void *listener_function(void * sock)
{
	int sk = (intptr_t) sock;
	while (1)
	{
		int s;
		s = accept(sk, NULL, NULL);

		if (s < 0) break;

		sem_wait(&sem_empty);
		pthread_mutex_lock(&mutex);

		request[in] = s;
		in = (in + 1) % MAX_REQUEST;

		pthread_mutex_unlock(&mutex);
		sem_post(&sem_full);
	}
}

// Process for taking threads from the listener and consuming them from buffer
void *consumer(void * to_consume)
{
	int worker = (intptr_t) to_consume;
	while (1)
	{
		int s;
		sem_wait(&sem_full);
		pthread_mutex_lock(&mutex);

		s = request[out];
		printf("\nWorker %d Handling Request %d\n", worker, s);
		out = (out + 1) % MAX_REQUEST;

		pthread_mutex_unlock(&mutex);
		sem_post(&sem_empty);

		process(s);
	}
}

void multi_threaded_server(int sock, int n_threads)
{
	/* Initialize mutex lock and semaphores */
	pthread_mutex_init(&mutex, NULL);
  	sem_init(&sem_empty, 0, MAX_REQUEST);
	sem_init(&sem_full, 0, 0);

	/* Listener and Worker threads. */
	pthread_t workers[n_threads];
	pthread_t listener;

	int thread_status, i, j;

	pthread_create(&listener, NULL, listener_function, (void *) (intptr_t) sock);

	for (i=0; i<n_threads; i++)
	{
		pthread_create(&workers[i], NULL, consumer, (void *) (intptr_t) i);
	}

	while (1)
	{
		for (j=0; j<n_threads; j++) {
			thread_status = pthread_tryjoin_np(workers[j], NULL);

			if (thread_status == EBUSY) {
		    	continue;
			}

			pthread_create(&workers[j], NULL, consumer, (void *) (intptr_t) j);
			sem_post(&sem_empty);
		}
	}
}



int main(int argc, char *argv[])
{
		int sock;
		int n_threads;

		/* Random seed */
		srand(getpid() + time(NULL));

		/* Set # of worker thread */
		n_threads = 0;
		if(argc > 1) {
				n_threads = atoi(argv[1]);
				if(n_threads > 100) n_threads = 100;
				if(n_threads < 0) {
						fprintf(stderr, "usage: ./webserver (#_of_threads) (crash_rate(%))\n");
				}
		}

		/* Set crash rate */
		if(n_threads > 0 && argc > 2) {
				CRASH = atoi(argv[2]);
				if(CRASH > 30) CRASH = 30;
				if(CRASH < 0) {
						fprintf(stderr, "usage: ./webserver (#_of_threads) (crash_rate(%))\n");
				}
				printf("[pid %d] CRASH RATE = %d\%\n", getpid(), CRASH);
		}

		/* Initialize a socket */
		sock = binding_and_listen();

		/* start the server */
		if(n_threads == 0) single_threaded_server(sock);
		else multi_threaded_server(sock, n_threads);

		close(sock);

		return 0;
}
