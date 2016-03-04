#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>

int number, work;
sem_t *ready, *mutex;

void thread_func(int id);
void server_func();

int main(int argc, char** argv) {
	pthread_t client1, client2, server;
	work = 0;
	ready = sem_open("/ready", O_CREAT, 0644, 0);
	mutex = sem_open("/mutex", O_CREAT, 0644, 1);
	pthread_create(&server, NULL, (void*) server_func,(void*) 0);
	pthread_create(&client1, NULL,(void*) thread_func,(void*) 1);
	pthread_create(&client2, NULL,(void*) thread_func,(void*) 2);
	int ret = 0;
	pthread_join(client1,(void**) ret);
	pthread_join(client2,(void**) ret);
	return 0;
}

void server_func() {
	while(1) {
		if(work) {
			number *= 2;
			work = 0;
			sem_post(ready);
		}
	}
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
		work = 1;
		sem_wait(ready);
		printf("id = %i  result = %i\n", id, number);
		sem_post(mutex);
	}
}
