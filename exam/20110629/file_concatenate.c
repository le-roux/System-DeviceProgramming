#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>

void* thread_func(void*);

typedef struct dir_t {
	DIR* dir;
	pthread_mutex_t dir_lock;
} dir_t;

dir_t* directory;
extern int errno;

int main(int argc, char* argv[]) {
	if(argc != 3) {
		printf("Usage : %s nthreads dirpath\n", argv[0]);
		return 1;
	}

	int K = atoi(argv[1]);
	directory = (dir_t*)malloc(sizeof(dir_t));
	directory->dir = opendir(argv[2]);
	chdir(argv[2]);	
	mkdir("./tmp", 0777);
	chdir("..");
	if (directory->dir == NULL) {
		printf("Error when opening directory\n");
		printf("errno = %s\n", strerror(errno));
		return 1;
	}
	pthread_mutex_init(&directory->dir_lock, NULL);
	pthread_t* threads[K];
	int* ti;
	for (int i = 0; i < K; i++) {
		printf("creating thread\n");
		threads[i] = (pthread_t*)malloc(sizeof(pthread_t));
		ti = (int*)malloc(sizeof(int));
		*ti = i;
		if(pthread_create(threads[i], NULL, thread_func, ti)){
			printf("Fail creating thread %i\n", i);
			return 1;
		};
	}
	while(1);
}

void* thread_func(void* arg) {
	struct dirent* filehead1 = (struct dirent*)malloc(sizeof(struct dirent));
	struct dirent* filehead2 = (struct dirent*)malloc(sizeof(struct dirent));
	int file1, file2;
	pthread_mutex_lock(&directory->dir_lock);
	while ((filehead1 = readdir(directory->dir)) && (filehead2 = readdir(directory->dir))) {
		if(filehead1 == NULL || filehead2 == NULL)
			printf("errno %s\n", strerror(errno));
		pthread_mutex_unlock(&directory->dir_lock);
		if(!strcmp(filehead1->d_name, ".") || !strcmp(filehead1->d_name, "..")) {
			pthread_mutex_lock(&directory->dir_lock);
			printf("1 try to read directory\n");
			continue;
		} else if(!strcmp(filehead2->d_name, ".") || !strcmp(filehead2->d_name, "..")) {
			pthread_mutex_lock(&directory->dir_lock);
			printf("2 try to read directory\n");
			continue;
		}
		file1 = open(filehead1->d_name, O_RDONLY);
		printf("opened %s\n", filehead1->d_name);
		file2 = open(filehead2->d_name, O_RDONLY);
		printf("opened %s\n", filehead2->d_name);
		unlink(filehead1->d_name);
		unlink(filehead2->d_name);
		int length = strlen(filehead1->d_name) + strlen(filehead2->d_name) + strlen("./tmp/");
		char filename[length];
		strcpy(filename, "./tmp/");
		strcat(filename, filehead1->d_name);
		strcat(filename, filehead2->d_name);
		printf("new file : %s\n", filename);
		
		close(file1);
		close(file2);
		pthread_mutex_lock(&directory->dir_lock);
	}
	pthread_mutex_unlock(&directory->dir_lock);
	return arg;
}
