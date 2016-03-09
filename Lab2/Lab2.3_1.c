#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char** argv) {
	if (argc != 2) {
		printf("Syntax : %s k\n", argv[0]);
		return 0;
	}
	
	int k = atoi(argv[1]);
	float *v1, *v2;
	float *mat[k];
	v1 = (float*) malloc(k * sizeof(float));
	v2 = (float*) malloc(k * sizeof(float));
	for (int i = 0; i < k; i++) {
		mat[i] = (float*) malloc(k * sizeof(float));
	}
	srand(time(NULL));
	for (int i = 0; i < k; i++) {
		v1[i] = ((float)(rand() % 100) / 100) - 0.5;
	}
	
	for (int i = 0; i < k; i++) {
		v2[i] = ((float)(rand() % 100) / 100) - 0.5;
	}

	for(int i = 0; i < k; i++) {
		for (int j = 0; j < k; j++) {
			mat[i][j] = ((float)(rand() % 100) / 100) - 0.5;
		}
	}

	//v = mat * v2
	float* v;
	v = (float*) malloc(k * sizeof(float));

	for (int i = 0; i < k; i++) {
		v[i] = 0;
		for (int j = 0; j < k; j++) {
			v[i]+= mat[i][j] * v2[i];
		}
	}

	//result = T(v1) * v
	float result = 0;
	for (int i = 0; i < k; i++) {
		result += v1[i] * v[i];
	}

	printf("result = %f\n", result);
	return 0;
}
