#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <algorithm> 

using namespace std;

struct employee {
    int num;
    char name[10];
    double hours;
};

bool Compare(const employee& a, const employee& b) {
    return a.num < b.num;
}

int main(int argc, char* argv[]) {

    setlocale(LC_ALL, "rus");

    string binfile = argv[1];
    string reportfile = argv[2];
    double salaryPerHour = atof(argv[3]);

    ifstream bin(binfile, ios::binary);
    if (!bin) {
        cerr << "Ошибка при открытии бинарного файла " << binfile << endl;
        return 1;
    }

    ofstream report(reportfile);
    if (!report) {
        cerr << "Ошибка при открытии бинарного файла " << reportfile << endl;
        return 1;
    }

    vector<employee> Employees;
    employee emp;

    while (bin.read(reinterpret_cast<char*>(&emp), sizeof(emp))) {
        Employees.push_back(emp);
    }

    sort(Employees.begin(), Employees.end(), Compare);

    report << "Отчет по файлу " << binfile << endl;
    report << "----------------------------------------" << endl;
    report << "Номер\tИмя\t\tЧасы\tЗарплата" << endl;
    report << "----------------------------------------" << endl;

    for (int i = 0; i < Employees.size(); i++) {
        double salary = Employees[i].hours * salaryPerHour;
        report << Employees[i].num << "\t" << Employees[i].name << "\t\t"
            << Employees[i].hours << "\t" << fixed << setprecision(2) << salary << endl;
    }

    Employees.clear();
    bin.close();
    report.close();
    return 0;
}
