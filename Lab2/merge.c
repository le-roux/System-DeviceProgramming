#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

int* vet;
sem_t* mutex;

void 
merge(int *vet, int left, int middle, int right) {
    int i, j, k;
    int n1 = middle - left + 1;
    int n2 =  right - middle;

    int L[n1], R[n2];  // temp arrays
    
    for (i = 0; i < n1; i++)  // make a copy
        L[i] = vet[left + i];
    for (j = 0; j < n2; j++)
        R[j] = vet[middle + 1 + j];
        
    // Merge the temp arrays in vet[l..r]
    i = 0; 
    j = 0; 
    k = left; // Initial index of merged subarray
    while (i < n1 && j < n2)  {
      if (L[i] <= R[j])
        vet[k++] = L[i++];
      else
        vet[k++] = R[j++];
    }
    // Copy the remaining elements of L[]
    while (i < n1)
      vet[k++] = L[i++];
    // Copy the remaining elements of R[]
    while (j < n2)
      vet[k++] = R[j++];
}
 
void mergeSort(int *vet, int left, int right){
int middle;

  if (left < right){
    middle = left + (right - left)/2;  // Same as (left + right)/2, but avoids overflow for large l and r
 
    mergeSort(vet, left, middle);
    mergeSort(vet, middle+1, right);
 
    merge(vet, left, middle, right);
  }
}

void threaded_sort(int* arg) {
	int left, right, middle;
	left = arg[0];
	right = arg[1];
	if (left < right) {
		middle = left + (right - left)/2;
		pthread_t thread1, thread2;
		int arg1[2] = {left, middle};
		pthread_create(&thread1, NULL,(void*) threaded_sort,(void*) arg1);
		int arg2[2] = {middle + 1, right};
		pthread_create(&thread2, NULL,(void*) threaded_sort,(void*) arg2);
	
		int* ret;
		pthread_join(thread1,(void**) ret);
		pthread_join(thread2,(void**) ret);
		sem_wait(mutex);
		merge(vet, left, middle, right);
		sem_post(mutex);
	}
}

int main(int argc, char ** argv) {
  int i, n, len;

  if (argc != 2) {
    printf ("Syntax: %s dimension", argv[0]);
    return (1);
  }
  
  n = atoi(argv[1]);

  vet = (int*) malloc(n * sizeof(int));
  
  mutex = (sem_t*) malloc(sizeof(sem_t));
  sem_init(mutex, 0, 1);
  sem_wait(mutex);
  for(i = 0;i < n;i++) {
    vet[i] = rand() % 100;
	printf("%d\n",vet[i]);
  }
  sem_post(mutex);
    int arg[2] = {0, n};
  threaded_sort(arg);

  printf("\n");
  sem_wait(mutex);
  for(i = 0;i < n;i++) 
	printf("%d\n",vet[i]);
  sem_post(mutex);
  return 0;
}
