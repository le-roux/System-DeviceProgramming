#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct {
	int cnt;
	pthread_mutex_t mutex;
} barrier;

void* thread_func(void* i);

barrier b;
float *v, *v1, *v2, *mat;
int k;

int main(int argc, char** argv) {
	if (argc != 2) {
		printf("Syntax : %s k\n", argv[0]);
		return 0;
	}

	k = atoi(argv[1]);
	//Create all the variables
	v1 = (float*) malloc(k * sizeof(float));
	v2 = (float*) malloc(k * sizeof(float));
	mat = (float*) malloc(k * k * sizeof(float));

	//Generate all the random numbers
	srand(time(NULL));
	for (int i = 0; i < k; i++) {
		v1[i] = ((float)(rand() % 100) / 100) - 0.5;
	}
	
	for (int i = 0; i < k; i++) {
		v2[i] = ((float)(rand() % 100) / 100) - 0.5;
	}

	for(int i = 0; i < k; i++) {
		for (int j = 0; j < k; j++) {
			mat[k * i + j] = ((float)(rand() % 100) / 100) - 0.5;
		}
	}

	pthread_mutex_lock(&b.mutex);
	b.cnt = k;
	pthread_mutex_unlock(&b.mutex);

	/*
	 * Work
	 */

	//v = mat * v2
	v = (float*) malloc(k * sizeof(float));
	pthread_t thread;
	int* pi;
	for (int i = 0; i < k; i++) {
		pi = (int*)malloc(sizeof(int));
		*pi = i;
		pthread_create(&thread, NULL, thread_func, pi);
	}
	pthread_exit(0);
}

void* thread_func(void* arg) {
	int* addr = arg;
	int i = *addr;
	v[i] = 0;
	for (int j = 0; j < k; j++) {
		v[i]+= mat[k * i + j] * v2[i];
	}
	pthread_mutex_lock(&b.mutex);
	b.cnt--;
	//Last thread to terminate
	if (b.cnt == 0) {
		//result = T(v1) * v
		float result = 0;
		for (int i = 0; i < k; i++) {
			result += v1[i] * v[i];
		}
		printf("result = %f\n", result);
	}
	pthread_mutex_unlock(&b.mutex);
	return addr;
}
