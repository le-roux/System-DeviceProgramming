#include <stdio.h>
#include <stdlib.h>

void exchange(int* a, int* b);
void sort(int* src, int size);
void display(int* src, int size);
void write(char* filename, int* src, int size);
void write_binary(char* filename, int* src, int size);
 
int main(int argc, char** argv) {
	if (argc != 3) {
		printf("Wrong number of arguments\n");
		return 1;
	}
	int n1, n2;
	n1 = atoi(argv[1]);
	n2 = atoi(argv[2]);
	int *v1, *v2;
	v1 = malloc(n1 * sizeof(int));
	v2 = malloc(n2 * sizeof(int));
	for (int i = 0; i < n1; i++) {
		*(v1 + i) = rand() % 91 + 10;
	}
	for (int i = 0; i < n2; i++) {
		*(v2 + i) = rand() % 81 + 20;
	}
	sort(v1, n1);
	sort(v2, n2);
	write("fv1.txt", v1, n1);
	write("fv2.txt", v2, n2);
	write_binary("fv1.b", v1, n1);
	write_binary("fv2.b", v2, n2);
	return 0;
}

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

void display(int* src, int size) {
	for (int i = 0; i < size; i++) {
		printf("%i ", *(src + i));
	}
}

void write(char* filename, int* src, int size) {
	FILE* f = fopen(filename, "w");
	fwrite(src, sizeof(int), size,f);
	fclose(f); 
}

void write_binary(char* filename, int* src, int size) {
	FILE* f = fopen(filename, "wb");
	fwrite(src, sizeof(int), size, f);
	fclose(f);
}
