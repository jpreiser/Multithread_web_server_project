#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "webserver.h"

#define MAX_REQUEST 10

int request[MAX_REQUEST];

sem_t sem_full;
sem_t sem_empty;
pthread_mutex_t mutex;

void multi_threaded_server(int sock, int n_thread)
{
		/* Initialize mutex lock and semaphores */
		pthread_mutex_init(&mutex, NULL);
  sem_init(&sem_empty, 0, MAX_REQUEST);
  sem_init(&sem_full, 0, 0);
		
		/* Create a listener thread and "n_thread" worker threads. */
}

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


int main(int argc, char *argv[])
{
		int i, sock;
		int n_thread;

		/* Random seed */
		srand(getpid() + time(NULL));

		/* Set # of worker thread */
		n_thread = 0;
		if(argc > 1) {
				n_thread = atoi(argv[1]);
				if(n_thread > 100) n_thread = 100;
				if(n_thread < 0) {
						fprintf(stderr, "usage: ./webserver (#_of_threads) (crash_rate(%))\n");
				}
		}

		/* Set crash rate */
		if(n_thread > 0 && argc > 2) {
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
		if(n_thread == 0) single_threaded_server(sock);
		else multi_threaded_server(sock, n_thread);
		
		close(sock);

		return 0;
}

