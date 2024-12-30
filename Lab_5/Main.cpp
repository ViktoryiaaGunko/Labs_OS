#include <iostream>
#include <fstream>
#include <vector>
#include <windows.h>
#include <conio.h>
#include <algorithm>
#include "Header.h"
using namespace std;

HANDLE* hStarted;
HANDLE* hPipe;
HANDLE* hThreads;
HANDLE* hSemaphore;
employee* emps;

string file_name;
int number_of_employees;
int number_of_clients;

//обработка запросов клиентов
DWORD WINAPI operations(LPVOID pipe) {
    HANDLE hPipe = (HANDLE)pipe;
    DWORD dwRead; //чтение
    DWORD dwWrite; //запись

    //обработка запросов
    int mess;
    while (true) {
        if (!ReadFile(hPipe, &mess, sizeof(mess), &dwRead, NULL)) {
            cout << "Ошибка чтения данных из именованного канала.\n";
        }
        else {
            int ID = mess / 10;
            int chosen_option = mess % 10;
            if (chosen_option == 1 || chosen_option == 2) {
                WaitForSingleObject(hSemaphore[ID - 1], INFINITE);
                employee* emp_to_push = new employee();
                emp_to_push->num = emps[ID - 1].num;
                emp_to_push->hours = emps[ID - 1].hours;
                strcpy_s(emp_to_push->name, emps[ID - 1].name);

                bool checker = WriteFile(hPipe, emp_to_push, sizeof(employee), &dwWrite, NULL);
                if (checker) {
                    cout << "Данные были отправлены." << endl;
                }
                else {
                    cout << "Данные не были отправлены." << endl;
                }

                ReadFile(hPipe, emp_to_push, sizeof(employee), &dwWrite, NULL);
                emps[ID - 1].hours = emp_to_push->hours;
                strcpy_s(emps[ID - 1].name, emp_to_push->name);

                ofstream file(file_name);
                for (int i = 0; i < number_of_employees; i++) {
                    file << emps[i].num << " " << emps[i].name << " " << emps[i].hours << endl;
                }
                file.close();

                int msg;
                ReadFile(hPipe, &msg, sizeof(msg), &dwWrite, NULL);
                if (msg == 1) {
                    ReleaseSemaphore(hSemaphore[ID - 1], 1, NULL);
                }
            }
        }
    }
    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);
}

int main() {
    setlocale(LC_ALL, "RUS");
    cout << "Введите имя файла: ";
    cin >> file_name;

    cout << "Введите количество сотрудников: ";
    cin >> number_of_employees;

    //информация о сотрудниках
    emps = new employee[number_of_employees];
    for (int i = 0; i < number_of_employees; i++) {
        cout << "Введите ID сотрудника " << i + 1 << ": ";
        cin >> emps[i].num;
        cout << "Введите имя сотрудника:" << ": ";
        cin >> emps[i].name;
        cout << "Введите количество часов сотрудника:" << ": ";
        cin >> emps[i].hours;
    }

    ofstream file(file_name);
    for (int i = 0; i < number_of_employees; i++) {
        file << emps[i].num << " " << emps[i].name << " " << emps[i].hours << endl;
    }
    file.close();

    cout << "Введите количество клиентов:" << ": ";
    cin >> number_of_clients;

    hStarted = new HANDLE[number_of_clients];
    hSemaphore = new HANDLE[number_of_employees];

    //каждого сотрудника создаем семафор, инициализируя его значением, равным количеству клиентов
    for (int i = 0; i < number_of_employees; i++) {
        hSemaphore[i] = CreateSemaphoreA(NULL, number_of_clients, number_of_clients, "hSemaphore");
    }

    // Он позволяет создать несколько клиентских процессов. Каждый итерация соответствует одному клиенту
    for (int i = 0; i < number_of_clients; ++i) {
        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(STARTUPINFOA));
        si.cb = sizeof(STARTUPINFOA);
        char clientCmdLine[] = "Client.exe";

        if (!CreateProcessA(NULL, clientCmdLine, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
            cerr << "Не удалось создать процесс. Ошибка: " << GetLastError() << endl;
            return 1;
        }

        //событие сообщающее о запуске процесса
        hStarted[i] = CreateEventA(NULL, FALSE, FALSE, "Process Started");
        CloseHandle(pi.hProcess);
    }
    //ожидаем запуск всех клиентов (ожидаем пока все дискрипторы не станут сигнальными)
    WaitForMultipleObjects(number_of_clients, hStarted, TRUE, INFINITE);

    //массивы дескрипторов для именованных каналов и потоков
    hPipe = new HANDLE[number_of_clients];
    hThreads = new HANDLE[number_of_clients];

    //создаём именованные каналы для каждого клиента, которые позволят клиентам и серверу обмениваться данными
    for (int i = 0; i < number_of_clients; i++) {
        //PIPE_ACCESS_DUPLEX - канал будет использоваться в двунаправленном режиме: отправлены и получены через один и тот же канал
        hPipe[i] = CreateNamedPipeA("\\\\.\\pipe\\pipe_name", PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES, 0, 0, INFINITE, NULL);
        if (hPipe[i] == INVALID_HANDLE_VALUE) {
            cerr << "Ошибка создания именованного канала. Ошибка: " << GetLastError() << endl;
            return 1;
        }
        if (!ConnectNamedPipe(hPipe[i], (LPOVERLAPPED)NULL)) {
            cerr << "Ошибка подключения. Ошибка: " << GetLastError() << endl;
            CloseHandle(hPipe[i]);
            return 1;
        }
        // создаём новый поток для обработки операций с именованными каналами
        // это позволяет программе работать параллельно
        hThreads[i] = CreateThread(NULL, 0, operations, static_cast<LPVOID>(hPipe[i]), 0, NULL);
    }

    // программа не будет продолжать выполнение, пока все клиентские потоки не завершат свою работу
    WaitForMultipleObjects(number_of_clients, hThreads, TRUE, INFINITE);
    cout << "Все клиенты завершили свою работу." << endl;

    delete[] emps;
    delete[] hStarted;
    delete[] hPipe;
    delete[] hThreads;
    delete[] hSemaphore;

    cout << "Нажмите любую клавишу для выхода.";
    _getch();
    return 0;
}
