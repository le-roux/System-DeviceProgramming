#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include "office.h"

void* student(void*);
void* office(void*);
void* special_office(void*);

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
	srand(time(NULL));
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
	pthread_cond_init(buffer->notfull, NULL);
	pthread_cond_init(buffer->notempty, 0);
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
	while (buf->count == buf->dim) {
		pthread_cond_wait(buf->notfull, &buf->lock);
	}
	buf->buffer[buf->in] = info;
	buf->in++;
	//TO DO : in case of buffer overflow
	buf->count++;
	pthread_cond_broadcast(buf->notempty);
	pthread_mutex_unlock(&buf->lock);
}

Info receive(Buffer* buf) {
	pthread_mutex_lock(&buf->lock);
	while (buf->count == 0) {
		pthread_cond_wait(buf->notempty, &buf->lock);
	}
	Info info = buf->buffer[buf->out];
	buf->out++;
	//TO DO : cyclic buffer
	buf->count--;
	pthread_cond_broadcast(buf->notfull);
	pthread_mutex_unlock(&buf->lock);
	return info;
}

void* office(void* arg) {
	int* office_number = (int*)arg;
	//Initialisation of the urgent queue for this office
	urgent_Q[*office_number] = B_init(k/2);

	while(1) {

	}
	return arg;
}

void* special_office(void* arg) {
	Info info;
	while(1) {
		info = receive(special_Q);
		printf("student %i served by the special office\n", info.id);
		sleep(rand() % 4 + 3);
		info.urgent = 0; //not sure
		send(answer_Q[info.id], info);
	}

	return arg;
}

void* student(void* arg) {
	int* student_number = (int*)arg;
	//Initialisation of the answer queue for this student
	answer_Q[student_number] = B_init(3);
	volatile Info info = {*student_number, NUM_OFFICES, 0};

	sleep(rand() % 9 + 1);
	
	//Go in the normal queue
	send(normal_Q, info);
	//Wait until an office served us
	info = receive(answer_Q[info.id]);
	printf("student %i terminated after service at office %i\n", info.id, info.office_no); 
	
	if (info.urgent == 1) {
		//This student needs additional information
		//Go to the special queue
		send(special_Q, info);
		//Wait until special office has served us
		info = receive(answer_Q[info.id]);
		printf("student %i served by the special office\n", info.id);
		//Go to the urgent queue of the proper office
		send(urgent_Q[info.office_no], info);
		//Wait until the office has served us
		info = receive(answer_Q[info.id]);
		printf("student %i completed at office %i\n", info.id, info.office_no);
	} else {
		//nothing to do
	}
	printf("student %i terminated after service at office %i\n", info.id, info.office_no);
	//TO DO : check if it's the last student to terminate
	return arg;
}
