#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
using namespace std;

void writeRandomNumber(const string& filePath, int length) {
    string digits(length, '0');
    for (int i = 0; i < length; i++) {
        digits[i] = char('0' + rand() % 10);
    }
    if (length > 0) {
        digits[length - 1] = char('1' + rand() % 9);
    }
    ofstream out(filePath, ios::binary);
    out << length << '\n' << digits << '\n';
}

int main() {
    int firstLength, secondLength;
    cout << "N1 = ";
    cin >> firstLength;
    cout << "N2 = ";
    cin >> secondLength;
    srand(time(nullptr));

    writeRandomNumber("Numar1.txt", firstLength);
    writeRandomNumber("Numar2.txt", secondLength);

    cout << "Generated Numar1.txt and Numar2.txt\n";
    return 0;
}
