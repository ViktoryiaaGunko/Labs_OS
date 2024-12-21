#include <conio.h>
#include <windows.h>
#include <iostream>
#include "Header.h"
using namespace std;

int main() {
    setlocale(LC_ALL, "RUS");

    //подключение к именованному каналу
    HANDLE hPipe = CreateFileA("\\\\.\\pipe\\pipe_name", GENERIC_READ | GENERIC_WRITE,
        0, NULL, OPEN_EXISTING, 0, NULL);

    if (hPipe == INVALID_HANDLE_VALUE) {
        cerr << "Ошибка создания именованного канала." << endl;
        return GetLastError(); 
    }

    while (true) {
        int ID, action = 0;
        cout << "Модификация - 1 " << endl << "Прочитать данные - 2 " << endl << "Выход - 3";
        cin >> action;

        if (action == 3) {
            break;
        }

        cout << "Введите ID сотрудника:\n>>>";
        cin >> ID; 

        int mes = ID * 10 + action; // Формируем сообщение для отправки

        DWORD dwWritten; // количествo записанных байтов
        //oтправляем сообщение серверу
        WriteFile(hPipe, &mes, sizeof(mes), &dwWritten, NULL);

        //модификация
        if (action == 1) { 
            employee emp; 
            // чтение данные сотрудника с сервера
            ReadFile(hPipe, &emp, sizeof(employee), &dwWritten, NULL);

            // текущие данные 
            cout << "ID: " << emp.num << ", Имя: " << emp.name << ", Часы: " << emp.hours << endl;
            cout << "Введите новое имя:" << endl;
            cin >> emp.name; 
            cout << "Введите новые часы:" << endl;
            cin >> emp.hours; 
            // отправка обновленных данных обратно на сервер
            WriteFile(hPipe, &emp, sizeof(employee), &dwWritten, NULL);
        }
        //чтение
        else if (action == 2) { 
            employee emp; 
            // чтение данных сотрудника с сервера
            ReadFile(hPipe, &emp, sizeof(employee), &dwWritten, NULL);
            cout << "ID: " << emp.num << ", Имя: " << emp.name << ", Часы: " << emp.hours << endl;
        }
    }

    CloseHandle(hPipe); 
    return 0; 
}
