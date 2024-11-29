#include <iostream>
#include <fstream>
#include <string>

using namespace std;

struct employee {
    int num;
    char name[10];
    double hours;
};

int main(int argc, char* argv[]) {

    setlocale(LC_ALL, "rus");

    string binfile = argv[1];
    int n = atoi(argv[2]);

    ofstream bin(binfile, ios::binary);
    if (!bin) {
        cerr << "ќшибка при открытии файла " << binfile << endl;
        return 1;
    }

    for (int i = 0; i < n; i++) {
        employee emp;
        cout << "¬ведите номер сотрудника: ";
        cin >> emp.num;
        cout << "¬ведите им€ сотрудника: ";
        bin >> emp.name;
        cout << "¬ведите количество отработанных часов: ";
        cin >> emp.hours;

        bin.write(reinterpret_cast<char*>(&emp), sizeof(employee));
    }

    bin.close();
    return 0;
}
