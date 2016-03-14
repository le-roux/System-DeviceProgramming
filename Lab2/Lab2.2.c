#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct Data {
	int* vet;
	pthread_mutex_t mutex;
} Data;

int threshold;
Data data;

void 
merge(int *vet, int left, int middle, int right) {
    int i, j, k;
    int n1 = middle - left + 1;
    int n2 =  right - middle;
    int L[n1], R[n2];  // temp arrays
    for (i = 0; i <= n1; i++)  // make a copy
        L[i] = vet[left + i];
    for (j = 0; j <= n2; j++)
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
 
void threaded_sort(int* arg) {
	int left, right, middle;
	left = arg[0];
	right = arg[1];
	if (left < right) {
		middle = left + (right - left)/2;
		if ((right - left) >= threshold) {
			pthread_t thread1, thread2;
			int arg1[2] = {left, middle};
			pthread_create(&thread1, NULL,(void*) threaded_sort,(void*) arg1);
			int arg2[2] = {middle + 1, right};
			pthread_create(&thread2, NULL,(void*) threaded_sort,(void*) arg2);
	
			int* ret;
			pthread_join(thread1,(void**) ret);
			pthread_join(thread2,(void**) ret);
		} else {
			int arg1[2] = {left, middle};
			int arg2[2] = {middle + 1, right};
			threaded_sort(arg1);
			threaded_sort(arg2);
		}
		
		pthread_mutex_lock(&data.mutex);
		merge(data.vet, left, middle, right);
		pthread_mutex_unlock(&data.mutex);
	}
}

int main(int argc, char ** argv) {
  int i, n, len;

  if (argc != 3) {
    printf ("Syntax: %s dimension threshold\n", argv[0]);
    return (1);
  }
  
  n = atoi(argv[1]);
  threshold = atoi(argv[2]);

  data.vet = (int*) malloc(n * sizeof(int));
  
  pthread_mutex_init(&data.mutex, NULL);
  pthread_mutex_lock(&data.mutex);
  for(i = 0;i < n;i++) {
    data.vet[i] = rand() % 100;
	printf("%d\n",data.vet[i]);
  }
  pthread_mutex_unlock(&data.mutex);
  int arg[2] = {0, n-1};
  threaded_sort(arg);

  printf("\n");
  pthread_mutex_lock(&data.mutex);
  for(i = 0;i < n;i++) 
	printf("%d\n",data.vet[i]);
  pthread_mutex_unlock(&data.mutex);
  return 0;
}
