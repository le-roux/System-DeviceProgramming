#include <stdlib.h>
#include <stdio.h>
#include <time.h>


int main(int argc, char* argv[]) {
	if (argc != 3) {
		printf("Syntax : %s n output_filename\n", argv[0]);
		return 1;
	}
	int n = atoi(argv[1]);
	srand(time(NULL));
	FILE* f = fopen(argv[2], "wb");
	for (int i = 0; i < n; i++) {
		int a = rand() % 100;
		fwrite(&a, sizeof(int), 1, f);
		printf("%i ", a);
	}
	fclose(f);
	return 0;
}
