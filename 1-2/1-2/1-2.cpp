#include <iostream>
#include <stdio.h>
#include <string>
#include <fstream>
#include <cctype>
#include <Windows.h>
#include <sstream>
using namespace std;

#define data_line 10
#define word 200

/*
for (int i = 0; i < line_count; ++i) {
				for (int y = 0; y < lines[i].length(); ++y) {

				}
			}

*/


void paintdata(int line_count, string lines[],int onoffh) {
	if (onoffh == 1) {
		for (int i = 0; i < line_count; ++i) {
			for (int y = 0; y < lines[i].length(); ++y) {
				cout << lines[i][y];

				if (lines[i][y] >= '0' && lines[i][y] <= '9') {
					cout << endl;
				}
			}

			cout << endl;
		}
	}
	else if (onoffh == 0) {
		for (int i = 0; i < line_count; i++) {
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
			cout << lines[i] << endl;
		}
	}
}

int main() {
	ifstream file("data.txt");
	if (!file) {
		cout << "파일을 열수 없습니다.\n";
		return 1;
	}

	string lines[data_line];
	int line_count = 0;
	int onoffc = 0;
	int onoffd = 0;
	int onoffe = 0;
	int onofff = 0;
	int onoffg = 0;
	int onoffh = 0;

	while (line_count < data_line && getline(file, lines[line_count])) {
		++line_count;
	}
	cout << "data.txt\n";
	for (int i = 0; i < line_count; i++) {
		cout << lines[i] << endl;
	}

	cout << "\n명령어를 입력하세요\n";
	cout << "a: 대소문자 변환\n";
	cout << "b: 단어 개수 출력\n";
	cout << "c: 대문자 색 변환 및 개수 출력\n";
	cout << "d: 각 문장 별로 거꾸로 출력하기\n";
	cout << "e: 모든 공백에 * 삽입\n";
	cout << "f: 공백 기준 모든 단어 거꾸로 출력\n";
	cout << "g: 특정 문자 다른 문자로 변경\n";
	cout << "h: 숫자 뒤에 문장 줄 넘김\n";
	cout << "i: 입력받은 단어 찾아 색 변경 및 개수 출력\n";
	cout << "j: 문장의 순서 변경하여 출력\n";
	cout << "q:프로그램 종료\n";
	string command;
	string oldword;
	string newword;
	string order;
	string findword;

	while (true) {
		cin >> command;

		if (command == "a") {
			for (int i = 0; i < line_count; ++i) {
				for (int y = 0; y < lines[i].length(); ++y) {
					if (lines[i][y] >= 'A' && lines[i][y] <= 'Z') {
						lines[i][y] = lines[i][y] + 32;
					}
					else if (lines[i][y] >= 'a' && lines[i][y] <= 'z') {
						lines[i][y] = lines[i][y] - 32;
					}
				}
				cout << lines[i] << endl;
			}
			onoffc = 0;
		}

		else if (command == "b") {
			int blank = 0;
			for (int i = 0; i < line_count; ++i) {
				blank = 0;
				for (int y = 0; y < lines[i].length(); ++y) {
					if (lines[i][y] == ' ') {
						blank++;
					}
				}
				cout << lines[i] << " 단어수: " << blank + 1 << endl;
			}
			onoffc = 0;
		}
		else if (command == "c") {
			if (onoffc == 0) {
				int count = 0;
				onoffc = 1;
				for (int i = 0; i < line_count; ++i) {
					for (int y = 0; y < lines[i].length(); ++y) {
						if ((y == 0 || lines[i][y - 1] == ' ') &&
							(lines[i][y] >= 'A' && lines[i][y] <= 'Z')) {
							count++;
							SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
							while (y < lines[i].length() && lines[i][y] != ' ') {
								cout << lines[i][y];
								y++;
							}
							SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
							y--;
						}
						else {
							cout << lines[i][y];
						}
					}

					cout << endl;
				}
				cout << "대문자로 시작하는 단어 개수: " << count << endl;
			}
			else {
				for (int i = 0; i < line_count; i++) {
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
					cout << lines[i] << endl;
				}
				onoffc = 0;
			}
		}
		else if (command == "d") {
			if (onoffd == 0) {
				onoffd = 1;
				for (int i = 0; i < line_count; ++i) {
					for (int y = lines[i].length(); y >= 0; --y) {
						cout << lines[i][y];
					}
					cout << endl;
				}
			}
			else {
				onoffd = 0;
				paintdata(line_count, lines, onoffh);
			}
		}
		else if (command == "e") {
			if (onoffe == 0) {
				onoffe = 1;
				for (int i = 0; i < line_count; ++i) {
					for (int y = lines[i].length(); y >= 0; --y) {
						if (lines[i][y] == ' ') {
							lines[i][y] = '*';
						}
					}
					cout << lines[i] << endl;
				}
			}
			else if (onoffe == 1) {
				onoffe = 0;
				for (int i = 0; i < line_count; ++i) {
					for (int y = lines[i].length(); y >= 0; --y) {
						if (lines[i][y] == '*') {
							lines[i][y] = ' ';
						}
					}
					cout << lines[i] << endl;
				}
			}
		}
		else if (command == "f") {
			if (onofff == 0) {
				onofff = 1;
				for (int i = 0; i < line_count; ++i) {
					int start = 0;

					for (int y = 0; y <= lines[i].length(); ++y) {
						if (y == lines[i].length() || lines[i][y] == ' ') {
							int end = y - 1;

							for (int k = end; k >= start; --k) {
								cout << lines[i][k];
							}

							if (y != lines[i].length()) {
								cout << ' ';
							}

							start = y + 1;
						}
						else if (y == lines[i].length() || lines[i][y] == '*') {
							int end = y - 1;

							for (int k = end; k >= start; --k) {
								cout << lines[i][k];
							}

							if (y != lines[i].length()) {
								cout << '*';
							}

							start = y + 1;
						}

					}

					cout << endl;
				}
			}
			else {
				onofff = 0;
				paintdata(line_count, lines, onoffh);
			}
		}
		else if (command == "g") {
			if (onoffg == 0) {
				onoffg = 1;
				cout << "바꿀문자: ";
				cin >> oldword;

				cout << "새롭게 입력할 문자: ";
				cin >> newword;

				for (int i = 0; i < line_count; ++i) {
					for (int y = 0; y < lines[i].length(); ++y) {
						if (oldword[0] == lines[i][y]) {
							lines[i][y] = newword[0];
						}
					}
				}
				paintdata(line_count, lines, onoffh);
			}
			else if (onoffg == 1) {
				onoffg = 0;
				for (int i = 0; i < line_count; ++i) {
					for (int y = 0; y < lines[i].length(); ++y) {
						if (newword[0] == lines[i][y]) {
							lines[i][y] = oldword[0];
						}
					}
				}
				paintdata(line_count, lines, onoffh);
			}

		}
		else if (command == "h") {
			if (onoffh == 0) {
				onoffh = 1;
				paintdata(line_count, lines, onoffh);
			}
			else {
				onoffh = 0;
				paintdata(line_count, lines, onoffh);
			}
		}
		else if (command == "i") {
			int count = 0;
			cout << "찾을 단어: ";
			cin >> findword;

			for (int k = 0; k < findword.length(); ++k) {
				findword[k] = tolower(findword[k]);
			}

			for (int i = 0; i < line_count; ++i) {
				int start = 0;
				for (int y = 0; y < lines[i].length(); ++y) {
					if (y == lines[i].length() || lines[i][y] == ' ') {
						int end = y - 1;

						string temp = "";
						for (int k = start; k <= end; ++k) {
							temp += lines[i][k];
						}

						for (int k = 0; k < temp.length(); ++k) {
							temp[k] = tolower(temp[k]);
						}

						if (temp == findword) {
							count++;
							SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
							for (int k = start; k <= end; ++k) {
								cout << lines[i][k];
							}
							SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
						}

						else {
							for (int k = start; k <= end; ++k) {
								cout << lines[i][k];
							}
						}

						if (y != lines[i].length()) {
							cout << ' ';
						}
						
						start = y + 1;
					}
				}
				cout << endl;
			}
			cout << "찾은 단어 개수: " << count << endl;


		}
		else if (command == "j") {
			string first = lines[0];

			for (int i = 0; i < line_count - 1; ++i) {
				lines[i] = lines[i + 1];
			}

			lines[line_count - 1] = first;
			paintdata(line_count, lines, onoffh);
		}
		else if (command == "k") {
			string last = lines[line_count - 1];

			for (int i = line_count - 1; i > 0; --i) {
				lines[i] = lines[i - 1];
			}

			lines[0] = last;
			paintdata(line_count, lines, onoffh);
		}
		else if (command == "q") {
			return 0;
		}

	}
}
