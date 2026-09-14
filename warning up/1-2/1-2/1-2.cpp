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


bool isDelimiter(char ch) {
	return ch == ' ' || ch == '*';
}

string lowerString(string text) {
	for (int i = 0; i < (int)text.length(); ++i) {
		if (text[i] >= 'A' && text[i] <= 'Z') {
			text[i] = text[i] + 32;
		}
	}

	return text;
}

string reverseLine(string text) {
	string result = "";

	for (int i = (int)text.length() - 1; i >= 0; --i) {
		result += text[i];
	}

	return result;
}

string reverseWords(string text) {
	string result = "";
	int start = 0;
	int len = (int)text.length();

	for (int y = 0; y <= len; ++y) {
		if (y == len || text[y] == ' ') {
			int end = y - 1;

			for (int k = end; k >= start; --k) {
				result += text[k];
			}

			if (y != len) {
				result += ' ';
			}

			start = y + 1;
		}
	}

	return result;
}

string makeDisplayLine(string line, int onoffd, int onoffe, int onofff, int onoffg, string oldword, string newword) {
	if (onoffg == 1 && oldword != "" && newword != "") {
		for (int i = 0; i < (int)line.length(); ++i) {
			if (line[i] == oldword[0]) {
				line[i] = newword[0];
			}
		}
	}

	if (onofff == 1) {
		line = reverseWords(line);
	}

	if (onoffd == 1) {
		line = reverseLine(line);
	}

	if (onoffe == 1) {
		for (int i = 0; i < (int)line.length(); ++i) {
			if (line[i] == ' ') {
				line[i] = '*';
			}
		}
	}

	return line;
}

void printChar(char ch, int onoffh) {
	cout << ch;

	if (onoffh == 1 && ch >= '0' && ch <= '9') {
		cout << endl;
	}
}

void printDisplayLine(string line, int onoffc, int onoffh, string findword, int& upperCount, int& findCount) {
	int y = 0;
	int len = (int)line.length();

	while (y < len) {
		if (isDelimiter(line[y])) {
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
			printChar(line[y], onoffh);
			++y;
		}
		else {
			int start = y;

			while (y < len && !isDelimiter(line[y])) {
				++y;
			}

			int end = y - 1;
			string temp = "";

			for (int k = start; k <= end; ++k) {
				temp += line[k];
			}

			temp = lowerString(temp);

			bool upperWord = onoffc == 1 && line[start] >= 'A' && line[start] <= 'Z';
			bool findWord = findword != "" && temp == findword;

			if (upperWord) {
				++upperCount;
			}

			if (findWord) {
				++findCount;
			}

			if (upperWord || findWord) {
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
			}
			else {
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
			}

			for (int k = start; k <= end; ++k) {
				printChar(line[k], onoffh);
			}

			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
		}
	}
}

int countWords(string line) {
	int count = 0;
	int inWord = 0;

	for (int i = 0; i < (int)line.length(); ++i) {
		if (line[i] != ' ' && inWord == 0) {
			++count;
			inWord = 1;
		}
		else if (line[i] == ' ') {
			inWord = 0;
		}
	}

	return count;
}

void paintdata(int line_count, string lines[], int onoffc, int onoffd, int onoffe, int onofff, int onoffg, int onoffh, string oldword, string newword) {
	int upperCount = 0;
	int findCount = 0;

	for (int i = 0; i < line_count; i++) {
		string line = makeDisplayLine(lines[i], onoffd, onoffe, onofff, onoffg, oldword, newword);
		printDisplayLine(line, onoffc, onoffh, "", upperCount, findCount);
		cout << endl;
	}

	if (onoffc == 1) {
		cout << "대문자로 시작하는 단어 개수: " << upperCount << endl;
	}
}

void paintdataWithFind(int line_count, string lines[], int onoffc, int onoffd, int onoffe, int onofff, int onoffg, int onoffh, string oldword, string newword, string findword) {
	int upperCount = 0;
	int findCount = 0;

	findword = lowerString(findword);

	for (int i = 0; i < line_count; i++) {
		string line = makeDisplayLine(lines[i], onoffd, onoffe, onofff, onoffg, oldword, newword);
		printDisplayLine(line, onoffc, onoffh, findword, upperCount, findCount);
		cout << endl;
	}

	if (onoffc == 1) {
		cout << "대문자로 시작하는 단어 개수: " << upperCount << endl;
	}

	cout << "찾은 단어 개수: " << findCount << endl;
}

int main() {
	string data;
	cout << "파일 이름을 입력하세요\n";
	cin >> data;

	ifstream file(data);
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
	cout << data << endl;
	paintdata(line_count, lines, onoffc, onoffd, onoffe, onofff, onoffg, onoffh, "", "");
	
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
	cout << "k: 문장의 순서 반대로 변경하여 출력\n";
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
			}
			paintdata(line_count, lines, onoffc, onoffd, onoffe, onofff, onoffg, onoffh, oldword, newword);
		}

		else if (command == "b") {
			int upperCount = 0;
			int findCount = 0;

			for (int i = 0; i < line_count; ++i) {
				string line = makeDisplayLine(lines[i], onoffd, onoffe, onofff, onoffg, oldword, newword);
				printDisplayLine(line, onoffc, onoffh, "", upperCount, findCount);
				cout << " 단어수: " << countWords(lines[i]) << endl;
			}

			if (onoffc == 1) {
				cout << "대문자로 시작하는 단어 개수: " << upperCount << endl;
			}
		}
		else if (command == "c") {
			onoffc = 1 - onoffc;
			paintdata(line_count, lines, onoffc, onoffd, onoffe, onofff, onoffg, onoffh, oldword, newword);
		}
		else if (command == "d") {
			onoffd = 1 - onoffd;
			paintdata(line_count, lines, onoffc, onoffd, onoffe, onofff, onoffg, onoffh, oldword, newword);
		}
		else if (command == "e") {
			onoffe = 1 - onoffe;
			paintdata(line_count, lines, onoffc, onoffd, onoffe, onofff, onoffg, onoffh, oldword, newword);
		}
		else if (command == "f") {
			onofff = 1 - onofff;
			paintdata(line_count, lines, onoffc, onoffd, onoffe, onofff, onoffg, onoffh, oldword, newword);
		}
		else if (command == "g") {
			if (onoffg == 0) {
				onoffg = 1;
				cout << "바꿀문자: ";
				cin >> oldword;

				cout << "새롭게 입력할 문자: ";
				cin >> newword;
			}
			else if (onoffg == 1) {
				onoffg = 0;
			}

			paintdata(line_count, lines, onoffc, onoffd, onoffe, onofff, onoffg, onoffh, oldword, newword);

		}
		else if (command == "h") {
			onoffh = 1 - onoffh;
			paintdata(line_count, lines, onoffc, onoffd, onoffe, onofff, onoffg, onoffh, oldword, newword);
		}
		else if (command == "i") {
			cout << "찾을 단어: ";
			cin >> findword;

			paintdataWithFind(line_count, lines, onoffc, onoffd, onoffe, onofff, onoffg, onoffh, oldword, newword, findword);
		}
		else if (command == "j") {
			if (line_count == 0) {
				paintdata(line_count, lines, onoffc, onoffd, onoffe, onofff, onoffg, onoffh, oldword, newword);
				continue;
			}

			string first = lines[0];

			for (int i = 0; i < line_count - 1; ++i) {
				lines[i] = lines[i + 1];
			}

			lines[line_count - 1] = first;
			paintdata(line_count, lines, onoffc, onoffd, onoffe, onofff, onoffg, onoffh, oldword, newword);
		}
		else if (command == "k") {
			if (line_count == 0) {
				paintdata(line_count, lines, onoffc, onoffd, onoffe, onofff, onoffg, onoffh, oldword, newword);
				continue;
			}

			string last = lines[line_count - 1];

			for (int i = line_count - 1; i > 0; --i) {
				lines[i] = lines[i - 1];
			}

			lines[0] = last;
			paintdata(line_count, lines, onoffc, onoffd, onoffe, onofff, onoffg, onoffh, oldword, newword);
		}
		else if (command == "q") {
			return 0;
		}

	}
}
