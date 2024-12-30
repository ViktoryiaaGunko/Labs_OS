#include <conio.h>
#include <windows.h>
#include <iostream>
#include "Header.h"

using std::cout;
using std::cin;

int main() {
    // событие будет использоваться для уведомления сервера о том, что клиент запущен
    HANDLE hStartEvent = OpenEventA(EVENT_MODIFY_STATE, FALSE, "Process Started");
    if (hStartEvent == NULL) {
        cout << "Ошибка события запуска\n";
        return GetLastError();
    }
    //Устанавливаем состояние события, уведомляя сервер о том, что клиент запущен
    SetEvent(hStartEvent);

    //Открываем именованный канал для обмена информацикй с сервером с разрешениями на чтение и запись
    HANDLE hPipe = CreateFileA("\\\\.\\pipe\\pipe_name", GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hPipe == INVALID_HANDLE_VALUE) {
        cout << "Ошибка создания именованного канала. Ошибка: " << GetLastError() << "\n";
        return GetLastError();
    }

    while (true) {
        int action = 0;
        cout << "Изменить данные - 1; \nЧитать данные - 2; \nВыход - 3 \n>>>";
        cin >> action;

        if (action == 3) {
            break;
        }
        else {
            DWORD dwWritten;
            DWORD dwReaden;
            int ID;
            cout << "Введите ID сотрудника\n>>>";
            cin >> ID;

            int new_mess = ID * 10 + action;

            bool a = WriteFile(hPipe, &new_mess, sizeof(new_mess), &dwWritten, NULL);
            if (!a) {
                cout << "Сообщение не было отправлено.\n";
            }

            employee* emp = new employee();
            ReadFile(hPipe, emp, sizeof(employee), &dwReaden, NULL);
            if (action == 1) {
                cout << "ID сотрудника: " << emp->num << ".\nИмя сотрудника: " << emp->name << ".\nЧасы сотрудника: " << emp->hours << ".\n";
                cout << "Введите новое имя:\n";
                cin >> emp->name;
                cout << "Введите новые часы:\n";
                cin >> emp->hours;
                a = WriteFile(hPipe, emp, sizeof(employee), &dwWritten, NULL);
            }
            else if (action == 2) {
                cout << "ID сотрудника: " << emp->num << ".\nИмя сотрудника: " << emp->name << ".\nЧасы сотрудника: " << emp->hours << ".\n";
            }

            if (a) {
                cout << "Сообщение было отправлено.\n";
            }
            else {
                cout << "Сообщение не было отправлено.\n";
            }

            new_mess = 1;
            WriteFile(hPipe, &new_mess, sizeof(new_mess), &dwWritten, NULL);
        }
    }

    return 0;
}
