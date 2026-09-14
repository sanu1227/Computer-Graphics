#include <iostream>
#include <stdio.h>
#include <string>
#include <cmath>
#include <algorithm>
using namespace std;

#define SIZE 10

typedef struct point {
	int x;
	int y;
	int z;
	int spc;
	int id;
}point;



typedef struct pointInfo {
	point p;
	int number;
}pointInfo;

point points[SIZE];
void printall() {
	for (int i = 0; i < SIZE; ++i) {
		cout << i << ": ";
		if (points[i].spc != 0) {
			printf("%d %d %d", points[i].x, points[i].y, points[i].z);
		}
		cout << endl;
	}
}

double distance(point a, point b) {
	int dx = a.x - b.x;
	int dy = a.y - b.y;
	int dz = a.z - b.z;

	return sqrt(dx * dx + dy * dy + dz * dz);
}

double originDistance(point p) {
	return sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
}

int main() {
	int count = 0;
	int nextId = 1;

	cout << "명령어를 입력하세요\n";
	cout << "+: 리스트의 맨 위 입력\n";
	cout << "-: 리스트 맨 위에서 삭제\n";
	cout << "e: 리스트 맨 아래에 입력\n";
	cout << "d: 리스트의 맨 아래 삭제\n";
	cout << "a: 리스트에 저장된 점의 개수 출력\n";
	cout << "b: 점들의 리스트 위치를 한 칸씩 내린다\n";
	cout << "c: 리스트를 비운다\n";
	cout << "f: 각 점에서 원점과의 거리를 계산 후 오름차순 정렬하여 출력\n";
	cout << "g: 두 점간의 거리를 계산하고 가장 먼 점과 가까운 점을 출력한다. 출력한 두 점과의 거리도 출력한다\n";
	cout << "q: 프로그램 종료\n";

	string command;
	while (true) {

		cin >> command;
		if (command == "e") {
			if (count >= SIZE) {
				cout << "리스트가 가득 차 있습니다\n";
				continue;
			}

			int x, y, z;
			cout << "x:";
			cin >> x;
			cout << "y:";
			cin >> y;
			cout << "z:";
			cin >> z;

			int emptyIndex = -1;

			for (int i = 0; i < SIZE; ++i) {
				if (points[i].spc == 0) {
					emptyIndex = i;
					break;
				}
			}

			for (int i = emptyIndex; i > 0; --i) {
				points[i] = points[i - 1];
			}

			points[0] = { x,y,z,1,nextId };
			nextId++;
			count++;

			printall();
		}
		else if (command == "-") {

			if (count <= 0) {
				cout << "삭제할 점이 없습니다\n";
				continue;
			}

			for (int i = SIZE - 1; i >= 0; --i) {
				if (points[i].spc != 0) {
					points[i] = { 0,0,0,0,0 };
					count--;
					break;
				}
			}

			printall();
		}
		else if (command == "+") {
			if (count >= SIZE) {
				cout << "리스트가 가득 차 있습니다\n";
				continue;
			}


			int x, y, z;
			cout << "x:";
			cin >> x;
			cout << "y:";
			cin >> y;
			cout << "z:";
			cin >> z;

			for (int i = 0; i < SIZE; ++i) {
				if (points[i].spc == 0) {
					points[i] = { x,y,z,1,nextId };
					nextId++;
					count++;
					break;
				}
			}
			printall();
		}
		else if (command == "d") {
			if (count <= 0) {
				cout << "삭제할 점이 없습니다\n";
				continue;
			}

			for (int i = 0; i < SIZE; ++i) {
				if (points[i].spc != 0) {
					points[i] = { 0,0,0,0,0 };
					count--;
					break;
				}
			}
			printall();
		}
		else if (command == "a") {
			cout << "저장된 점의 개수: " << count << endl;
			printall();
		}
		else if (command == "b") {
			if (count < 1) {
				continue;
			}
			point temp = points[0];
			for (int i = 1; i < SIZE; ++i) {
				if (i > 0) {
					points[i-1] = points[i];
				}
			}

			points[9] = temp;

			printall();
		}
		else if (command == "c") {
			for (int i = 0; i < SIZE; ++i) {
				points[i] = { 0,0,0,0,0 };
			}
			count = 0;
			nextId = 1;
			printall();
		}
		else if (command == "f") {
			pointInfo sorted[SIZE];
			int sortedCount = 0;

			for (int i = 0; i < SIZE; ++i) {
				if (points[i].spc != 0) {
					sorted[sortedCount].p = points[i];
					sorted[sortedCount].number = points[i].id;
					sortedCount++;
				}
			}

			sort(sorted, sorted + sortedCount, [](pointInfo a, pointInfo b) {
				return originDistance(a.p) < originDistance(b.p);
				});

			for (int i = 0; i < sortedCount; ++i) {
				cout << sorted[i].number << "번 점: "
					<< sorted[i].p.x << " "
					<< sorted[i].p.y << " "
					<< sorted[i].p.z
					<< " 원점 거리: " << originDistance(sorted[i].p) << endl;
			}
			printall();
		}
		else if (command == "g") {
			pointInfo valid[SIZE];
			int validCount = 0;

			for (int i = 0; i < SIZE; ++i) {
				if (points[i].spc != 0) {
					valid[validCount].p = points[i];
					valid[validCount].number = points[i].id;
					validCount++;
				}
			}

			if (validCount < 2) {
				cout << "점이 2개 이상 있어야 합니다\n";
				continue;
			}

			// 가장 가까운 거리와 가장 먼 거리를 저장할 변수이다.
			double minDistance = distance(valid[0].p, valid[1].p);
			double maxDistance = distance(valid[0].p, valid[1].p);

			// minA, minB는 valid 배열 안에서 현재까지 찾은 가장 가까운 두 점의 위치이다.
			// maxA, maxB는 valid 배열 안에서 현재까지 찾은 가장 먼 두 점의 위치이다.
			// 실제 출력 번호는 배열 위치가 아니라 점을 입력할 때 저장한 id를 사용한다.
			int minA = 0;
			int minB = 1;
			int maxA = 0;
			int maxB = 1;

			
			for (int i = 0; i < validCount - 1; ++i) {
				for (int j = i + 1; j < validCount; ++j) {
					double d = distance(valid[i].p, valid[j].p);

					cout << valid[i].number << "번 점과 " << valid[j].number << "번 점 사이 거리: " << d << endl;

					// 방금 구한 거리 d가 현재까지의 가장 가까운 거리보다 작으면
					// 가장 가까운 거리와 그 거리를 만든 두 점의 번호를 새 값으로 바꾼다.
					if (d < minDistance) {
						minDistance = d;
						minA = i;
						minB = j;
					}

					// 방금 구한 거리 d가 현재까지의 가장 먼 거리보다 크면
					// 가장 먼 거리와 그 거리를 만든 두 점의 번호를 새 값으로 바꾼다.
					if (d > maxDistance) {
						maxDistance = d;
						maxA = i;
						maxB = j;
					}
				}
			}

			cout << "가장 가까운 점: "
				<< valid[minA].number << "번 점과 " << valid[minB].number << "번 점, 거리: "
				<< minDistance << endl;

			cout << "가장 먼 점: "
				<< valid[maxA].number << "번 점과 " << valid[maxB].number << "번 점, 거리: "
				<< maxDistance << endl;
			printall();
		}
		else if (command == "q") {
			return 0;
		}
		else if (command == "p") {
			for (int i = 0; i < SIZE; ++i) {
				cout << i << ": ";
				if (points[i].spc != 0) {
					printf("%d %d %d", points[i].x, points[i].y, points[i].z);
				}
				cout << endl;
			}

		}
	}
}
