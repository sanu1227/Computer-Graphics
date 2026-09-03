#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <string>

using namespace std;

int det3(int m[3][3]) {
	return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
		- m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
		+ m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
}

int main() {
	const int SIZE = 4;
	string command;

	srand((unsigned int)time(NULL));

	int matrix[SIZE][SIZE];
	int matrix2[SIZE][SIZE];
	int matrixM[SIZE][SIZE];
	int matrixD[SIZE][SIZE];
	int matrixA[SIZE][SIZE];

	int onoff1 = 0;
	int onoff2 = 0;

	int mini1[SIZE] = { 0 };
	int mini2[SIZE] = { 0 };
	int max1[SIZE] = { 0 };
	int max2[SIZE] = {0};

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
	cout << "\n";

	while (true) {

		cout << "명령어를 입력하세요\n";
		cout << "m: 행렬의 곱셈\n";
		cout << "a: 행렬의 덧셈\n";
		cout << "d: 행렬의 뺄셈\n";
		cout << "r:행렬식의 값\n";
		cout << "t:전치 행렬\n";
		cout << "e:최소값 -\n";
		cout << "f: 최대값 +\n";
		cout << "+/-: 행렬의 모든 값에 +1/-1\n";
		cout << "s: 행렬의 값 새로 생성\n";
		cout << "q: 프로그램 종료\n";

		cin >> command;
		if (command == "m") {
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
		}

		else if (command == "a") {
			printf("\n");
			printf("matrixA\n");
			for (int row = 0; row < SIZE; ++row) {
				for (int col = 0; col < SIZE; ++col) {
					matrixA[row][col] = matrix[row][col] + matrix2[row][col];
					printf("%d ", matrixA[row][col]);
				}
				printf("\n");
			}
		}
		else if (command == "d") {
			printf("\n");
			printf("matrixD\n");
			for (int row = 0; row < SIZE; ++row) {
				for (int col = 0; col < SIZE; ++col) {
					matrixD[row][col] = matrix[row][col] - matrix2[row][col];
					printf("%d ", matrixD[row][col]);
				}
				printf("\n");
			}
		}
		else if(command == "r"){
			int sub1[3][3] = { matrix[1][1], matrix[1][2],matrix[1][3],
			matrix[2][1], matrix[2][2],matrix[2][3],
			matrix[3][1],matrix[3][2], matrix[3][3] };

			int sub2[3][3] = { matrix[1][0], matrix[1][2],matrix[1][3],
			matrix[2][0], matrix[2][2],matrix[2][3],
			matrix[3][0],matrix[3][2], matrix[3][3] };

			int sub3[3][3] = { matrix[1][0], matrix[1][1],matrix[1][3],
			matrix[2][0], matrix[2][1],matrix[2][3],
			matrix[3][0],matrix[3][1], matrix[3][3] };
			
			int sub4[3][3] = { matrix[1][0], matrix[1][1],matrix[1][2],
				matrix[2][0], matrix[2][1],matrix[2][2],
				matrix[3][0],matrix[3][1], matrix[3][2] };

			int R1 = matrix[0][0] * det3(sub1) - matrix[0][1] * det3(sub2) + matrix[0][2] * det3(sub3) - matrix[0][3] * det3(sub4);
			cout << "matrix1R: " << R1 << "\n";

			int sub21[3][3] = { matrix2[1][1], matrix2[1][2],matrix2[1][3],
			matrix2[2][1], matrix2[2][2],matrix2[2][3],
			matrix2[3][1],matrix2[3][2], matrix2[3][3] };

			int sub22[3][3] = { matrix2[1][0], matrix2[1][2],matrix2[1][3],
			matrix2[2][0], matrix2[2][2],matrix2[2][3],
			matrix2[3][0],matrix2[3][2], matrix2[3][3] };

			int sub23[3][3] = { matrix2[1][0], matrix2[1][1],matrix2[1][3],
			matrix2[2][0], matrix2[2][1],matrix2[2][3],
			matrix2[3][0],matrix2[3][1], matrix2[3][3] };

			int sub24[3][3] = { matrix2[1][0], matrix2[1][1],matrix2[1][2],
				matrix2[2][0], matrix2[2][1],matrix2[2][2],
				matrix2[3][0],matrix2[3][1], matrix2[3][2] };

			int R2 = matrix2[0][0] * det3(sub21) - matrix2[0][1] * det3(sub22) + matrix2[0][2] * det3(sub23) - matrix2[0][3] * det3(sub24);
			cout << "matrix2R: " << R2 << "\n";
		}
		else if (command == "t") {
			int matrixt1[SIZE][SIZE];
			int matrixt2[SIZE][SIZE];

			cout << "matrix1T\n";
			for (int row = 0; row < SIZE; ++row) {
				for (int col = 0; col < SIZE; ++col) {
					matrixt1[row][col] = matrix[col][row];
					cout << matrixt1[row][col] << " ";
				}
				cout << "\n";
			}

			cout << "matrix2T\n";
			for (int row = 0; row < SIZE; ++row) {
				for (int col = 0; col < SIZE; ++col) {
					matrixt2[row][col] = matrix2[col][row];
					cout << matrixt2[row][col] << " ";
				}
				cout << "\n";
			}

			int subt1[3][3] = { matrixt1[1][1], matrixt1[1][2],matrixt1[1][3],
matrixt1[2][1], matrixt1[2][2],matrixt1[2][3],
matrixt1[3][1],matrixt1[3][2], matrixt1[3][3] };

			int subt2[3][3] = { matrixt1[1][0], matrixt1[1][2],matrixt1[1][3],
			matrixt1[2][0], matrixt1[2][2],matrixt1[2][3],
			matrixt1[3][0],matrixt1[3][2], matrixt1[3][3] };

			int subt3[3][3] = { matrixt1[1][0], matrixt1[1][1],matrixt1[1][3],
			matrixt1[2][0], matrixt1[2][1],matrixt1[2][3],
			matrixt1[3][0],matrixt1[3][1], matrixt1[3][3] };

			int subt4[3][3] = { matrixt1[1][0], matrixt1[1][1],matrixt1[1][2],
				matrixt1[2][0], matrixt1[2][1],matrixt1[2][2],
				matrixt1[3][0],matrixt1[3][1], matrixt1[3][2] };

			int RT1 = matrixt1[0][0] * det3(subt1) - matrixt1[0][1] * det3(subt2) + matrixt1[0][2] * det3(subt3) - matrixt1[0][3] * det3(subt4);
			cout << "matrix1TR: " << RT1 << "\n";

			int subt21[3][3] = { matrixt2[1][1], matrixt2[1][2],matrixt2[1][3],
			matrixt2[2][1], matrixt2[2][2],matrixt2[2][3],
			matrixt2[3][1],matrixt2[3][2], matrixt2[3][3] };

			int subt22[3][3] = { matrixt2[1][0], matrixt2[1][2],matrixt2[1][3],
			matrixt2[2][0], matrixt2[2][2],matrixt2[2][3],
			matrixt2[3][0],matrixt2[3][2], matrixt2[3][3] };

			int subt23[3][3] = { matrixt2[1][0], matrixt2[1][1],matrixt2[1][3],
			matrixt2[2][0], matrixt2[2][1],matrixt2[2][3],
			matrixt2[3][0],matrixt2[3][1], matrixt2[3][3] };

			int subt24[3][3] = { matrixt2[1][0], matrixt2[1][1],matrixt2[1][2],
				matrixt2[2][0], matrixt2[2][1],matrixt2[2][2],
				matrixt2[3][0],matrixt2[3][1], matrixt2[3][2] };

			int RT2 = matrixt2[0][0] * det3(subt21) - matrix2[0][1] * det3(subt22) + matrix2[0][2] * det3(subt23) - matrix2[0][3] * det3(subt24);
			cout << "matrix2R: " << RT2 << "\n";
		}
		else if (command == "e") {
			if (onoff1 == 0) {
				onoff1 = 1;
				cout << "matrix1\n";
				for (int row = 0; row < SIZE; ++row) {
					mini1[row] = matrix[row][0];
					for (int col = 1; col < SIZE; ++col) {
						if (mini1[row] > matrix[row][col]) {
							mini1[row] = matrix[row][col];
						}
					}
				}
				for (int row = 0; row < SIZE; ++row) {
					for (int col = 0; col < SIZE; ++col) {
						matrix[row][col] -= mini1[row];
						printf("%d ", matrix[row][col]);
					}
					cout << "\n";
				}
				cout << "\nmatrix2\n";
				for (int row = 0; row < SIZE; ++row) {
					mini2[row] = matrix2[row][0];
					for (int col = 1; col < SIZE; ++col) {
						if (mini2[row] > matrix2[row][col]) {
							mini2[row] = matrix2[row][col];
						}
					}
				}
				for (int row = 0; row < SIZE; ++row) {
					for (int col = 0; col < SIZE; ++col) {
						matrix2[row][col] -= mini2[row];
						printf("%d ", matrix2[row][col]);
					}
					cout << "\n";
				}
			}
			else {
				onoff1 = 0;
				cout << "matrix1\n";
				for (int row = 0; row < SIZE; ++row) {
					for (int col = 0; col < SIZE; ++col) {
						matrix[row][col] += mini1[row];
						printf("%d ", matrix[row][col]);
					}
					cout << "\n";
				}
				cout << "\nmatrix2\n";
				for (int row = 0; row < SIZE; ++row) {
					for (int col = 0; col < SIZE; ++col) {
						matrix2[row][col] += mini2[row];
						printf("%d ", matrix2[row][col]);
					}
					cout << "\n";
				}
			}
		}
		else if (command == "f") {
			if (onoff2 == 0) {
				onoff2 = 1;
				cout << "matrix1\n";
				for (int row = 0; row < SIZE; ++row) {
					max1[row] = matrix[row][0];
					for (int col = 1; col < SIZE; ++col) {
						if (max1[row] < matrix[row][col]) {
							max1[row] = matrix[row][col];
						}
					}
				}
				for (int row = 0; row < SIZE; ++row) {
					for (int col = 0; col < SIZE; ++col) {
						matrix[row][col] += max1[row];
						printf("%d ", matrix[row][col]);
					}
					cout << "\n";
				}
				cout << "\nmatrix2\n";
				for (int row = 0; row < SIZE; ++row) {
					max2[row] = matrix2[row][0];
					for (int col = 1; col < SIZE; ++col) {
						if (max2[row] < matrix2[row][col]) {
							max2[row] = matrix2[row][col];
						}
					}
				}
				for (int row = 0; row < SIZE; ++row) {
					for (int col = 0; col < SIZE; ++col) {
						matrix2[row][col] += max2[row];
						printf("%d ", matrix2[row][col]);
					}
					cout << "\n";
				}
			}
			else {
				onoff2 = 0;
				cout << "matrix1\n";
				for (int row = 0; row < SIZE; ++row) {
					for (int col = 0; col < SIZE; ++col) {
						matrix[row][col] -= max1[row];
						printf("%d ", matrix[row][col]);
					}
					cout << "\n";
				}
				cout << "\nmatrix2\n";
				for (int row = 0; row < SIZE; ++row) {
					for (int col = 0; col < SIZE; ++col) {
						matrix2[row][col] -= max2[row];
						printf("%d ", matrix2[row][col]);
					}
					cout << "\n";
				}
			}
		}
		else if (command == "+") {
			cout << "matrix1\n";
			for (int row = 0; row < SIZE; ++row) {
				for (int col = 0; col < SIZE; ++col) {
					matrix[row][col] += 1;
					if (matrix[row][col] > 9) {
						matrix[row][col] = 0;
					}
					cout << " " << matrix[row][col];
				}
				cout << "\n";
			}
			cout << "matrix2\n";
			for (int row = 0; row < SIZE; ++row) {
				for (int col = 0; col < SIZE; ++col) {
					matrix2[row][col] += 1;
					if (matrix2[row][col] > 9) {
						matrix2[row][col] = 0;
					}
					cout << " " << matrix2[row][col];
				}
				cout << "\n";
			}
		}
		else if (command == "-") {
			cout << "matrix1\n";
			for (int row = 0; row < SIZE; ++row) {
				for (int col = 0; col < SIZE; ++col) {
					matrix[row][col] -= 1;
					if (matrix[row][col] < 0) {
						matrix[row][col] = 9;
					}
					cout << " " << matrix[row][col];
				}
				cout << "\n";
			}
			cout << "matrix2\n";
			for (int row = 0; row < SIZE; ++row) {
				for (int col = 0; col < SIZE; ++col) {
					matrix2[row][col] -= 1;
					if (matrix2[row][col] < 0) {
						matrix2[row][col] = 9;
					}
					cout << " " << matrix2[row][col];
				}
				cout << "\n";
			}
		}
		else if (command == "s") {
			onoff1 = 0;
			onoff2 = 0;

			for (int i = 0; i < SIZE; ++i) {
				mini1[i] = 0;
				mini2[i] = 0;
				max1[i] = 0;
				max2[i] = 0;
			}
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
		}
		else if (command == "q") {
			return 0;
		}
	}
	
	return 0;
}
