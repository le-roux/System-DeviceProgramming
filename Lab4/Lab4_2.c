#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include <signal.h>

typedef struct Region {
	//Size of a normal region. Note that the last one may have a different length.
	int region_length;
	int* left;
	int* right;
} Region;

typedef struct Barrier {
	pthread_mutex_t lock;
	int counter;
	sem_t sem;
	sem_t sem2;
} Barrier;

void* thread_sort(void*);

Barrier barrier;
int nthreads, *paddr, n;

int main(int argc, char* argv[]) {
	if (argc != 2) {
		printf("Syntax : %s filename\n", argv[0]);
		return 1;
	}
	//Open the file
	int fd;
	if ((fd = open(argv[1], O_RDWR)) == -1) {
		printf("Fail opening file\n");
		return 1;
	}

	//Get infos about the file
	struct stat stat_buf;
	if (fstat(fd, &stat_buf) == -1) {
		printf("Fail executing stat\n");
		return 1;
	}
	//Length of the file, in bytes
	int len = stat_buf.st_size;
	//Number of int in the file
	n = len / sizeof(int);

	//Mapping of the file in memory
	paddr = (int*) mmap((void*)0, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, (off_t)0);
	
	if(paddr == (int*)-1) {
		printf("Fail while mapping the file in memory\n");
		return 1;
	}
	close(fd);
	
	nthreads = ceil(log10(n));
	printf("nthreads = %i\n", nthreads);
	
	//Length of a region (except the last one)
	int region_length = n / nthreads;

	//Initialisation of the barrier
	pthread_mutex_init(&barrier.lock, NULL);
	pthread_mutex_lock(&barrier.lock);
	barrier.counter = nthreads;
	sem_init(&barrier.sem, 0, 0);
	sem_init(&barrier.sem2, 0, 0);
	pthread_mutex_unlock(&barrier.lock);
	
	//Creation of the regions & launching of the threads
	Region *regions[nthreads];
	pthread_t threads[nthreads];
	for (int i = 0; i < nthreads; i++) {
		regions[i] = (Region*)malloc(sizeof(Region));
		regions[i]->region_length = region_length;
		regions[i]->left = paddr + i * region_length;
		if (i == nthreads - 1) {
			regions[i]->right = paddr + n - 1;
		} else {
			regions[i]->right = paddr + (i + 1) * region_length - 1;
		}
		if (pthread_create(&threads[i], NULL, thread_sort, regions[i]) != 0) {
			printf("Fail creating thread\n");
			return 1;
		}
	}

	void* ret = NULL;
	pthread_exit(ret);
	return 0;
}

//Swaps two numbers
void swap(int* a, int* b) {
	int tmp = *a;
	*a = *b;
	*b = tmp;
}

void* thread_sort(void* arg) {
	Region* region = (Region*)arg;
	while (1) {
		int* cur = region->left;
		//Sort the region
		while(cur <= region->right) {
			int* tmp = cur;
			while(tmp >= region->left + 1 && *(tmp - 1) > *tmp) {
				swap(tmp - 1, tmp);
				tmp--;
			}
			cur++;
		}
		//Barrier
		pthread_mutex_lock(&barrier.lock);
		barrier.counter--;
		if (barrier.counter == 0) {
			//Last thread to reach the barrier
			//Swap borders
			int swapped = 0;
			for (int i = 1; i < nthreads; i++) {
				if (*(paddr + i * region->region_length - 1) > *(paddr + i * region->region_length)) {
					//A swap at a border is necessary
					swap(paddr + i * region->region_length - 1, paddr + i * region->region_length);
					swapped = 1;
				}
			}
			if (swapped) {
				//A swap at a border occured, regions must be sorted again
				for (int i = 0; i < nthreads; i++) {
					sem_post(&barrier.sem);
				}
			} else {
				//The file is fully sorted
				cur = paddr;
				int i = 0;
				while (cur < paddr + n) {
					printf("%i : %i\n", i, *cur);
					cur++;
					i++;
				}
				kill(getpid(), SIGTERM);
			}
		}
		pthread_mutex_unlock(&barrier.lock);
		
		sem_wait(&barrier.sem);
		
		//second part of the barrier, needed because threads loop
		pthread_mutex_lock(&barrier.lock);
		barrier.counter++;
		if (barrier.counter == nthreads) {
			for (int i = 0; i < nthreads; i++) {
				sem_post(&barrier.sem2);
			}
		}
		pthread_mutex_unlock(&barrier.lock);
		sem_wait(&barrier.sem2);
	}
	return arg;
}
