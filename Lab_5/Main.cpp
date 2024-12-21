#include <iostream>
#include <fstream>
#include <vector>
#include <windows.h>
#include <conio.h>
#include "Header.h"
using namespace std;

HANDLE hPipe;   //дескриптор именованного канала
employee* emps; //указатель на массив сотрудников
int number;   
string file;     

DWORD WINAPI operations(LPVOID pipe) {
    HANDLE hPipe = (HANDLE)pipe; // дескриптор канала
    DWORD dwRead, dwWrite; 
    int message; 

    while (true) {
        // Читаем сообщение из именованного канала
        if (!ReadFile(hPipe, &message, sizeof(message), &dwRead, NULL)) {
            cerr << "Ошибка чтения данных из именованного канала." << endl;
            break; 
        }

        int ID = message / 10; 
        int action = message % 10; 

        //модификация
        if (action == 1) { // Изменение данных сотрудника
            employee* new_emp = new employee(emps[ID - 1]); 
            if (WriteFile(hPipe, new_emp, sizeof(employee), &dwWrite, NULL)) {
                cout << "Данные для изменения отправлены." << endl;
            }
            else {
                cerr << "Данные для изменения не были отправлены." << endl;
            }

            // Читаем измененные данные сотрудника обратно от клиента
            ReadFile(hPipe, new_emp, sizeof(employee), &dwWrite, NULL);
            emps[ID - 1] = *new_emp; 

            // Сохраняем обновленные данные в файл
            ofstream file(file);
            for (int i = 0; i < number; i++) {
                file << emps[i].num << " " << emps[i].name << " " << emps[i].hours << endl;
            }

            delete new_emp; 
        }
        //чтение
        else if (action == 2) { 
            employee* new_emp = new employee(emps[ID - 1]); 
            if (WriteFile(hPipe, new_emp, sizeof(employee), &dwWrite, NULL)) {
                cout << "Данные для чтения отправлены." << endl;
            }
            else {
                cerr << "Данные для чтения не были отправлены." << endl;
            }
            delete new_emp; // Освобождаем выделенную память
        }
    }

    DisconnectNamedPipe(hPipe); //oтключаем именованный канал
    CloseHandle(hPipe); 
    return 0; 
}

int main() {
    setlocale(LC_ALL, "RUS");

    cout << "Введите имя файла: " << endl;
    cin >> file; 
    cout << "Введите количество сотрудников: " << endl;
    cin >> number; 
    emps = new employee[number];
    for (int i = 0; i < number; i++) {
        cout << "Введите ID сотрудника " << i + 1 << ": "<< endl;
        cin >> emps[i].num; 
        cout << "Введите имя сотрудника: " << endl;
        cin >> emps[i].name; 
        cout << "Введите часы работы сотрудника: " << endl;
        cin >> emps[i].hours; 
    }

    // Запись данных сотрудников в файл
    ofstream file(file);
    for (int i = 0; i < number; i++) {
        file << emps[i].num << " " << emps[i].name << " " << emps[i].hours << endl;
    }
    file.close(); 

    //создаем именованный канал, который будет использоваться для обмена сообщениями между сервером и клиентом  
    hPipe = CreateNamedPipeA("\\\\.\\pipe\\pipe_name", PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        1, sizeof(employee), sizeof(employee), 0, NULL);

    if (hPipe == INVALID_HANDLE_VALUE) {
        cerr << "Ошибка создания именованного канала." << endl;
        return GetLastError(); 
    }
    cout << "Ожидание подключения клиента." << endl;
    // блокируем выполнение программы до тех пор, пока клиент не подключится к каналу
    if (!ConnectNamedPipe(hPipe, NULL)) {
        cerr << "Ошибка подключения." << endl;
        CloseHandle(hPipe); 
        return GetLastError(); 
    }

    DWORD threadId; // ID потока
    // поток для обработки операций с клиентом
    CreateThread(NULL, 0, operations, (LPVOID)hPipe, 0, &threadId);

    cout << "Нажмите любую клавишу для выхода." << endl;
    _getch(); //oжидаем ввода от пользователя для выхода

    delete[] emps; 
    CloseHandle(hPipe); 

    return 0; 
}
