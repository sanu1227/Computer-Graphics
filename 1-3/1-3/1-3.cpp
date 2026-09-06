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
}point;

typedef struct pointInfo {
	point p;
	int number;
}pointInfo;

point points[SIZE];

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
		if (command == "+") {
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

			for (int i = count; i > 0; --i) {
				points[i] = points[i - 1];
			}

			points[0] = { x,y,z };
			count++;
		}
		else if (command == "-") {

			if (count <= 0) {
				cout << "삭제할 점이 없습니다\n";
				continue;
			}

			for (int i = 0; i < count - 1; ++i) {
				points[i] = points[i + 1];
			}
			count--;
		}
		else if (command == "e") {
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

			points[count] = { x,y,z };
			count++;
		}
		else if (command == "d") {
			if (count <= 0) {
				cout << "삭제할 점이 없습니다\n";
				continue;
			}

			points[count - 1] = { 0,0,0 };
			count--;
		}
		else if (command == "a") {
			cout << "저장된 점의 개수: " << count << endl;
		}
		else if (command == "b") {
			if (count <= 1) {
				continue;
			}

			point temp = points[count - 1];
			for (int i = count - 1; i > 0; --i) {
				points[i] = points[i - 1];
			}

			points[0] = temp;
		}
		else if (command == "c") {
			for (int i = 0; i < SIZE; ++i) {
				points[i] = { 0,0,0 };
			}
			count = 0;
		}
		else if (command == "f") {
			pointInfo sorted[SIZE];

			for (int i = 0; i < count; ++i) {
				sorted[i].p = points[i];
				sorted[i].number = i + 1;
			}

			sort(sorted, sorted + count, [](pointInfo a, pointInfo b) {
				return originDistance(a.p) < originDistance(b.p);
				});

			for (int i = 0; i < count; ++i) {
				cout << sorted[i].number << "번 점: "
					<< sorted[i].p.x << " "
					<< sorted[i].p.y << " "
					<< sorted[i].p.z
					<< " 원점 거리: " << originDistance(sorted[i].p) << endl;
			}
		}
		else if (command == "g") {
			if (count < 2) {
				cout << "점이 2개 이상 있어야 합니다\n";
				continue;
			}

			// 가장 가까운 거리와 가장 먼 거리를 저장할 변수이다.
			double minDistance = distance(points[0], points[1]);
			double maxDistance = distance(points[0], points[1]);

			// minA, minB는 현재까지 찾은 가장 가까운 두 점의 배열 인덱스이다.
			// maxA, maxB는 현재까지 찾은 가장 먼 두 점의 배열 인덱스이다.
			// 배열은 0번부터 시작하지만, 출력할 때는 사용자가 보기 쉽게 +1 해서 출력한다.
			int minA = 0;
			int minB = 1;
			int maxA = 0;
			int maxB = 1;

			
			for (int i = 0; i < count - 1; ++i) {
				for (int j = i + 1; j < count; ++j) {
					double d = distance(points[i], points[j]);

					cout << i + 1 << "번 점과 " << j + 1 << "번 점 사이 거리: " << d << endl;

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
				<< minA + 1 << "번 점과 " << minB + 1 << "번 점, 거리: "
				<< minDistance << endl;

			cout << "가장 먼 점: "
				<< maxA + 1 << "번 점과 " << maxB + 1 << "번 점, 거리: "
				<< maxDistance << endl;
		}
		else if (command == "q") {
			return 0;
		}
		else if (command == "p") {
			for (int i = 0; i < count; ++i) {
				printf("%d %d %d\n", points[i].x, points[i].y, points[i].z);
			}
		}
	}
}
