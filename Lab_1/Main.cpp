#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>

using namespace std;

struct employee {
    int num;
    char name[10];
    double hours;
};

void process(const wstring& command) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);

    LPWSTR command_lpwstr = const_cast<LPWSTR>(command.c_str());

    if (!CreateProcess(NULL, command_lpwstr, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        cerr << L"Ошибка при создании процесса." << endl;
        return;
    }
    else {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}

void printBinaryFile(const wstring& filename) {
    ifstream binFile(filename, ios::binary);

    employee emp;
    wcout << "Содержимое файла " << filename << endl;
    cout << "------------------------------" << endl;
    cout << "Номер\tИмя\t\tЧасы" << endl;
    cout << "------------------------------" << endl;

    while (binFile.read(reinterpret_cast<char*>(&emp), sizeof(employee))) {
        cout << emp.num << "\t" << emp.name << "\t\t" << emp.hours << endl;
    }

    binFile.close();
}

void printReportFile(const wstring& filename) {
    ifstream reportFile(filename);

    string line;
    while (getline(reportFile, line)) {
        cout << line << endl;
    }

    reportFile.close();
}

int main() {
    setlocale(LC_ALL, "rus");
    wstring binfile, reportfile;
    int n;
    double salaryPerHour;

    cout << "Введите название бинарного файла: ";
    wcin >> binfile;
    cout << "Введите количество записей: ";
    cin >> n;

    wstring creator = L"Creator.exe " + binfile + L" " + to_wstring(n);
    process(creator);

    printBinaryFile(binfile);

    cout << "Введите название бинарного файла отчета: ";
    wcin >> reportfile;
    cout << "Введите оплату за час работы: ";
    cin >> salaryPerHour;

    wstring reporter = L"Reporter.exe " + binfile + L" " + reportfile + L" " + to_wstring(salaryPerHour);
    process(reporter);

    printReportFile(reportfile);

    return 0;
}