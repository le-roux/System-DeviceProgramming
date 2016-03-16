#include <stdio.h>
#include <pthread.h>
#include "office.h"

void* student(void*);
void* office(void*);

int main(int argc, char* argv[]) {
	if (argc != 2) {
		printf("Syntax : %s number_of_students\n", argv[0]);
		return 1;
	}
	pthread_mutex_init(&num_students.lock, NULL);
	pthread_mutex_lock(&num_students.lock);
	num_students.num = atoi(argv[1]);
	pthread_mutex_unlock(&num_students.lock);
	k = atoi(argv[1]);

	//Creation of the queues
	normal_Q = B_init(k);
	special_Q = B_init(k);
	urgent_Q = (Buffer**)malloc(sizeof(Buffer*) * NUM_OFFICES);
	answer_Q = (Buffer**)malloc(sizeof(Buffer*) * k);

	int* pi;
	//Creation of the students threads
	pthread_t students_threads[k];
	for (int i = 0; i < k; i++) {
		pi = (int*)malloc(sizeof(int));
		*pi = i;
		pthread_create(&students_threads[i], NULL, student, pi); 
	}

	//Creation of the offices threads
	pthread_t offices_threads[NUM_OFFICES];
	for (int i = 0; i < NUM_OFFICES; i++) {
		pi = (int*)malloc(sizeof(int));
		*pi = i;
		pthread_create(&offices_threads[i], NULL, office, pi);
	}
	return 0;
}

Buffer* B_init(int dim) {
	Buffer* buffer = (Buffer*)malloc(sizeof(Buffer));
	buffer->buffer = (Info*)malloc(dim * sizeof(Info));
	pthread_mutex_init(&buffer->lock, NULL);
	buffer->notfull = cond_init(0);
	buffer->notempty = cond_init(0);
	buffer->in = 0;
	buffer->out = 0;
	buffer->count = 0;
	buffer->dim = dim;
	return buffer;
}

Cond* cond_init(int n) {
	Cond* cond = (Cond*)malloc(sizeof(Cond));
	pthread_mutex_init(&cond->lock, NULL);
	pthread_cond_init(cond->cond, NULL);
	*cond->urgent = 0;
	cond->normal = n;
	return cond;
}

void send (Buffer* buf, Info info) {
	pthread_mutex_lock(&buf->lock);
	buf->buffer[buf->in] = info;
	buf->in++;
	//TO DO : in case of buffer overflow
	pthread_mutex_unlock(&buf->lock);
}

Info receive(Buffer* buf) {
	pthread_mutex_lock(&buf->lock);
	Info info = buf->buffer[buf->out];
	buf->out++;
	//TO DO : cyclic buffer
	pthread_mutex_unlock(&buf->lock);
	return info;
}

void* office(void* arg) {
	int* office_number = (int*)arg;
	//Initialisation of the urgent queue for this office
	urgent_Q[*office_number] = B_init(k/2);

	return arg;
}

void* student(void* arg) {
	int* student_number = (int*)arg;
	//Initialisation of the answer queue for this student
	answer_Q = B_init(3);

	return arg;
}
