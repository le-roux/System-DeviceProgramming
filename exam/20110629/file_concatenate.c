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
	void* ret;
	for (int i= 0; i < K; i++) {
		pthread_join(*threads[i], &ret);
	}
}

void* thread_func(void* arg) {
	struct dirent* filehead1 = (struct dirent*)malloc(sizeof(struct dirent));
	struct dirent* filehead2 = (struct dirent*)malloc(sizeof(struct dirent));
	int file1, file2;
	struct stat status;
	int last_errno = errno;
	int* index = (int*)arg;
		int cont = 1, next1 = 1, next2 = 1;
	while (cont) {
		pthread_mutex_lock(&directory->dir_lock);
		printf("thread %i enters CR\n", *index);
		int rewinded = 0;
		do {
			last_errno = errno;
			filehead1 = readdir(directory->dir);
			if (filehead1 == NULL) {
				if (errno == last_errno) {
					if(rewinded) {
						cont = 0;
						break;
					}
					rewinddir(directory->dir);
					rewinded = 1;
				} else {
					printf("reading filehead1 errno : %s\n", strerror(errno));
					cont = 0;
					next1 = 0;
				}
			} else if(!strcmp(filehead1->d_name,".") || !strcmp(filehead1->d_name,"..") || !strcmp(filehead1->d_name, "tmp")) {
				next1 = 1;
			} else
				next1 = 0;
		} while(next1);

		if (cont == 0)
			break;

		rewinded = 0;
		
		do {
			last_errno = errno;
			filehead2 = readdir(directory->dir);
			if (filehead2 == NULL) {
				if (errno == last_errno) {
					if(rewinded) {
						cont = 0;
						break;
					}
					rewinddir(directory->dir);
					rewinded = 1;
				} else {
					printf("reading filehead2 errno : %s\n", strerror(errno));
					cont = 0;
				}
			} else if (!strcmp(filehead2->d_name, ".") || !strcmp(filehead2->d_name, "..") || !strcmp(filehead2->d_name, "tmp") || !strcmp(filehead1->d_name, filehead2->d_name))
				next2 = 1;
			 else
				next2 = 0;
		} while(next2);
		
		if(cont == 0)
			break;

		printf("thread %i exits CR\n", *index);
		pthread_mutex_unlock(&directory->dir_lock);

		file1 = open(filehead1->d_name, O_RDONLY);
		if(file1 == -1)
			printf("pb opening file %s : errno = %s\n",filehead1->d_name, strerror(errno));
		fstat(file1, &status);
		if (status.st_mode & S_IFDIR) {
			printf("it's a directory\n");
			close(file1);
			continue;
		}
		printf("file 1 opened\n");
		file2 = open(filehead2->d_name, O_RDONLY);
		if (file2 == -1)
			printf("pb opening file %s : errno = %s\n", filehead2->d_name, strerror(errno));
		fstat(file2, &status);
		if (status.st_mode & S_IFDIR) {
			printf("It's a directory\n");
			close(file2);
			continue;
		}
		printf("unlink\n");
		unlink(filehead1->d_name);
		unlink(filehead2->d_name);
		int length = strlen(filehead1->d_name) + strlen(filehead2->d_name) + strlen("tmp/");
		char filename[length];
		strcpy(filename, "tmp/");
		strcat(filename, filehead1->d_name);
		strcat(filename, filehead2->d_name);
		printf("new file : %s\n", filename);
		int dest = open(filename, O_WRONLY | O_CREAT);
		if (dest == -1)
			printf("pb openging dest file : errno = %s\n", strerror(errno));
		char buffer[1024];
		int count;
		count = read(file1, buffer, 1024);
		while(count != 0 && count != -1) {
			write(dest, buffer, count);
			count = read(file1, buffer, 1024);
		}
		while((count = read(file2, buffer, 1024))) {
			write(dest, buffer, count);
		}
		close(dest);
		length = strlen(filehead1->d_name) + strlen(filehead2->d_name);
		char hardlink[length];
		strcpy(hardlink, filehead1->d_name);
		strcat(hardlink, filehead2->d_name);
		link(filename, hardlink);
		chmod(hardlink, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
		unlink(filename);
		close(file1);
		close(file2);
	}
	return arg;
}
