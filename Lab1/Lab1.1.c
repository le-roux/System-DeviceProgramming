#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void exchange(int* a, int* b);
void sort(int* src, int size);
void display(int* src, int size);
void write(char* filename, int* src, int size);
void write_binary(char* filename, int* src, int size);
 
int main(int argc, char** argv) {
	if (argc != 3) {
		printf("Wrong number of arguments\n Correct syntax is %s n1 n2", argv[0]);
		return 1;
	}
	int n1, n2;
	n1 = atoi(argv[1]);
	n2 = atoi(argv[2]);
	int *v1, *v2;
	v1 = malloc(n1 * sizeof(int));
	v2 = malloc(n2 * sizeof(int));
	//Fill the vectors with random numbers
	srand(time(NULL));
	for (int i = 0; i < n1; i++) {
		*(v1 + i) = rand() % 91 + 10;
	}
	for (int i = 0; i < n2; i++) {
		*(v2 + i) = rand() % 81 + 20;
	}
	sort(v1, n1);
	sort(v2, n2);
	//Write text files
	write("fv1.txt", v1, n1);
	write("fv2.txt", v2, n2);
	//Write binary files
	write_binary("fv1.b", v1, n1);
	write_binary("fv2.b", v2, n2);
	return 0;
}

/*
 * Sort a vector of int by increasing order
 * using the bubbles algorithm
 */
void sort(int* src, int size) {
	for (int i = 0; i < size; i++) {
		int tmp = i - 1;
		while ((*(src + tmp) > *(src + tmp + 1)) && tmp >= 0) {
			exchange(src + tmp, src + tmp + 1);
			tmp--;
		}
	}
}

void exchange (int* a, int* b) {
	int tmp = *a;
	*a = *b;
	*b = tmp;
}

//Print the content of a vector on the standard output
void display(int* src, int size) {
	for (int i = 0; i < size; i++) {
		printf("%i ", *(src + i));
	}
}

//Write the content of a vector in a text file
void write(char* filename, int* src, int size) {
	FILE* f = fopen(filename, "w");
	if (f != NULL) {
		fwrite(src, sizeof(int), size,f);
		fclose(f);
	}
}

//Write the content of a vector in a binary file
void write_binary(char* filename, int* src, int size) {
	FILE* f = fopen(filename, "wb");
	fwrite(src, sizeof(int), size, f);
	fclose(f);
}
