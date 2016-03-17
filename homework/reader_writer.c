/*
 * Simple implementation of the reader - writer paradigm
 * No precedence
 */

#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#define READER_NUM 10
#define WRITER_NUM 5

void* reader(void*);
void* writer(void*);

typedef struct Counter {
	int counter;
	pthread_mutex_t lock;
} Counter;

Counter* counter_init();
Counter *library, *count;

int main(int argc, char* argv[]) {
	if(argc != 1) {
		printf("Syntax : %s\n", argv[0]);
		return 1;
	}
	
	pthread_t threads_reader[READER_NUM];
	pthread_t threads_writer[WRITER_NUM];
	
	//Initialization of counters
	library = counter_init();
	count = counter_init();
	
	//Initialisation of pseudo-random generator
	srand(time(NULL));
	
	//Creation of all the threads
	int* pi;
	for (int i = 0; i < READER_NUM; i++) {
		pi = (int*)malloc(sizeof(int));
		*pi = i;
		pthread_create(&threads_reader[i], NULL, reader, pi);
	}

	for (int i = 0; i < WRITER_NUM; i++) {
		pi = (int*)malloc(sizeof(int));
		*pi = i;
		pthread_create(&threads_reader[i], NULL, writer, pi);
	}

	//Waiting for the termination of all the threads
	void* ret;
	for (int i = 0; i < READER_NUM; i++) {
		pthread_join(threads_reader[i], &ret);
	}
	
	for (int i = 0; i < WRITER_NUM; i++) {
		pthread_join(threads_writer[i], &ret);
	}

	return 0;
}

Counter* counter_init() {
	Counter* counter = (Counter*)malloc(sizeof(Counter));
	pthread_mutex_init(&counter->lock, NULL);
	pthread_mutex_lock(&counter->lock);
	counter->counter = 0;
	pthread_mutex_unlock(&counter->lock);
	return counter;
}

void* reader(void* arg) {
	int* i = arg;
	sleep(rand() % 6 + 1);
	//Request access
	printf("Reader %i request access\n", *i);
	pthread_mutex_lock(&count->lock);
	count->counter++;
	if (count->counter == 1) {
		//First reader to enter
		pthread_mutex_lock(&library->lock);
	}
	pthread_mutex_unlock(&count->lock);
	//Access granted
	printf("Access granted to reader %i\n", *i);
	/*
	 * CRITICAL REGION
	 */
	 sleep(rand() % 6 + 1);
	 printf("library->counter = %i\n", library->counter);
	pthread_mutex_lock(&count->lock);
	count->counter--;
	if(count->counter == 0) {
		//Last reader to terminate it's reading
		pthread_mutex_unlock(&library->lock);
	}
	pthread_mutex_unlock(&count->lock);
	return arg;
}

void* writer(void* arg) {
	int* i = arg;
	sleep(rand() % 6 + 1);
	//Request access
	printf("Writer %i requests access\n", *i);
	pthread_mutex_lock(&library->lock);
	//Access granted
	printf("Access granted for writer %i\n", *i);
	/*
	 * CRITICAL REGION
	 */
	 sleep(rand() % 6 + 1);
	 library->counter++;
	 printf("Value updated by %i\n", *i);
	 //Leave the library
	 pthread_mutex_unlock(&library->lock);

	return arg;
}
