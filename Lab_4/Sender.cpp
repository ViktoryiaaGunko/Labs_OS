#include<iostream>
#include<fstream>
#include<string>

#include<Windows.h>
#include "Header.h"
using namespace std;

int main(int argc, char* argv[]) {
	setlocale(LC_ALL, "Russian");
	fstream file;

	if (argc < 4) {
		cout << "Использование: Sender <file_name> <number_of_records> <sender_id>" << endl;
		return 1;
	}

	//Отправить процессу Receiver сигнал на готовность к работе.
    //событие -> процесс готов к работе
    HANDLE ready = OpenEvent(EVENT_MODIFY_STATE, FALSE, (LPWSTR)(to_wstring(atoi(argv[3])) + L"ready").c_str());
    //мьютекс -> успешная записать сообщение
	HANDLE mutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, (LPWSTR)L"mutex");
	//семафор -> контроль количества доступных мест для записи
	HANDLE semW = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, (LPWSTR)L"write_sem");
	//семафор -> контроль количества доступных сообщений для чтения (позволяя процессу Receiver знать, когда есть новое сообщение)
	HANDLE semR = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, (LPWSTR)L"read_sem");
	
	if (!ready || !semR || !semW || !mutex) {
		cout << "Ошибка при открытии объектов синхронизации. " << endl;
		return -1;
	}

	//все процессы Sender готовы
	//запускаем цикл для записи сообщений
	SetEvent(ready);
	int action;
	while (true) {
		cout << "1 - написать сообщение" << endl;
		cout << "0 - выход" << endl;
		cin >> action;
		if (action != 0 && action != 1) {
			cout << "Неверная команда." << "\n";
			continue;
		}

		if (action == 0) {
			break;
		}

		cin.ignore();
		string text;
		cout << "Введите текст сообщения: ";
		getline(cin, text);

		//перед записью сообщения процесс Sender:
		//ждёт пока будет свободное место для записи
		WaitForSingleObject(semW, INFINITE);
		//захватывает мьютекс, чтобы предотвратить доступ других процессов к файлу во время записи
		WaitForSingleObject(mutex, INFINITE);

		message mes(text);
		file.open(argv[1], ios::binary | ios::app);
		file << mes;
		file.close();

		//освобождаем мьютекс для дальнейшей работы с файлом
		ReleaseMutex(mutex);
		//увеличивается семафор semR, сигнализируя, что появилось новое сообщение для чтения
		ReleaseSemaphore(semR, 1, nullptr);
		cout << "Сообщение отправлено! " << endl;
	}

	CloseHandle(mutex);
	CloseHandle(ready);
	CloseHandle(semW);
	CloseHandle(semR);

	return 0;
}
