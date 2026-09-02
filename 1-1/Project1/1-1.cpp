#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
	const int SIZE = 4;

	srand((unsigned int)time(NULL));

	int matrix[SIZE][SIZE];
	int matrix2[SIZE][SIZE];
	int matrixM[SIZE][SIZE];

	printf("matrix1\n");
	for (int row = 0; row < SIZE; ++row) {
		for (int col = 0; col < SIZE; ++col) {
			matrix[row][col] = rand() % 9;
			printf("%d ", matrix[row][col]);
		}
		printf("\n");
	}

	printf("\n");
	printf("matrix2\n");
	for (int row = 0; row < SIZE; ++row) {
		for (int col = 0; col < SIZE; ++col) {
			matrix2[row][col] = rand() % 9;
			printf("%d ", matrix2[row][col]);
		}
		printf("\n");
	}

	printf("\n");
	printf("matrixM\n");
	for (int row = 0; row < SIZE; ++row) {
		for (int col = 0; col < SIZE; ++col) {
			matrixM[row][col] = 0;

			for (int k = 0; k < SIZE; ++k) {
				matrixM[row][col] += matrix[row][k] * matrix2[k][col];
			}

			printf("%d ", matrixM[row][col]);
		}
		printf("\n");
	}

	int matrixA[SIZE][SIZE];
	printf("\n");
	printf("matrixA\n");
	for (int row = 0; row < SIZE; ++row) {
		for (int col = 0; col < SIZE; ++col) {
			matrixA[row][col] = matrix[row][col] + matrix2[row][col];
			printf("%d ", matrixA[row][col]);
		}
		printf("\n");
	}

	int matrixD[SIZE][SIZE];
	printf("\n");
	printf("matrixD\n");
	for (int row = 0; row < SIZE; ++row) {
		for (int col = 0; col < SIZE; ++col) {
			matrixD[row][col] = matrix[row][col] - matrix2[row][col];
			printf("%d", matrixD);
		}
		printf("\n");
	}

	return 0;
}
