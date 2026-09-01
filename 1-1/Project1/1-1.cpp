#include <stdio.h>
#include <Windows.h>
#include <time.h>


int main() {
	srand((unsigned int)time(NULL));
	int matrix[4][4];
	int matrix2[4][4];
	int matrixM[4][4];
	int rand1;
	int rand2;

	printf("matrix1\n");
	for (int i = 0; i < 4; ++i) {
		for (int y = 0; y < 4; ++y) {
			rand1 = rand() % 9;
			matrix[i][y] = rand1;
			printf("%d ", matrix[i][y]);
		}
		printf("\n");
	}
	printf("\n");
	printf("matrix2\n");
	for (int i = 0; i < 4; ++i) {
		for (int y = 0; y < 4; ++y) {
			rand2 = rand() % 9;
			matrix2[i][y] = rand2;
			printf("%d ", matrix2[i][y]);
		}
		printf("\n");
	}

	printf("\n");
	printf("matrixM\n");
	for (int i = 0; i < 4; ++i) {
		for (int y = 0; y < 4; ++y) {
			matrixM[y][i] = matrix[y][0] * matrix2[0][i] + matrix[y][1] * matrix2[1][i] +
				matrix[y][2] * matrix2[2][i] + matrix[y][3] * matrix2[3][i];
			printf("%d ", matrixM[y][i]);
		}
		printf("\n");
	}

	printf("\n");
	printf("matrixA\n");
	int matrixA[4][4];

}