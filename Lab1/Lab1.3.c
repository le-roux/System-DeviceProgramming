#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

int number, request_nb;
sem_t *ready, *mutex;

void thread_func(int id);
void server_func(int sig_nb);

int main(int argc, char** argv) {
	pthread_t client1, client2;
	request_nb = 0;
	ready = (sem_t*) malloc(sizeof(sem_t));
	sem_init(ready, 0, 0);
	mutex = (sem_t*) malloc(sizeof(sem_t));
	sem_init(mutex, 0, 1);
	signal(SIGUSR1, server_func);
	
	pthread_create(&client1, NULL,(void*) thread_func,(void*) 1);
	pthread_create(&client2, NULL,(void*) thread_func,(void*) 2);
	
	int ret = 0;
	pthread_join(client1,(void**) ret);
	pthread_join(client2,(void**) ret);
	printf("Number of requests : %i\n",request_nb);
	return 0;
}

void server_func(int sig_nb) {
	sem_wait(mutex);
	number *= 2;
	request_nb++;
	sem_post(mutex);
	sem_post(ready);
	signal(SIGUSR1, server_func);
}

void thread_func(int id) {
	FILE* f;
	if (id == 1) {
		f = fopen("fv1.b", "rb");
	} else {
		f = fopen("fv2.b", "rb");
	}
	if (f == NULL) {
		printf("Impossible to open file\n");
		return;
	}
	while (1) {	
		sem_wait(mutex);
		if(fread(&number, sizeof(int), 1, f) != 1) {
			sem_post(mutex);
			fclose(f);
			break;
		}
		sem_post(mutex);
		kill(getpid(), SIGUSR1);
		sem_wait(ready);
		sem_wait(mutex);
		printf("id = %i  result = %i\n", id, number);
		sem_post(mutex);
	}
}
