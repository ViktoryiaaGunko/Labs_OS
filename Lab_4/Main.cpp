#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <Windows.h>
#include"Header.h"
using namespace std;

/* start создаёт экземпляры процесса Sender, что позволяет запускать несколько процессов отправителя, 
которые могут одновременно отправлять сообщения в общий бинарный файл*/
HANDLE start(wstring commandL) {
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	//создаём новый процесс
	if (!CreateProcess(nullptr, (LPWSTR)commandL.c_str(), nullptr, nullptr,
		FALSE, CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &pi)) {
		return nullptr;
	}
	CloseHandle(pi.hThread);
	//возвращаем дескриптор нового процесса для последующего управления этим процессом
	return pi.hProcess;
}

int main() {
	setlocale(LC_ALL, "Russian");

	wstring nameFile;
	int numRecords, numSenders;

	cout << "Введите имя бинарного файла: ";
	wcin >> nameFile;
	fstream file(nameFile, ios::binary | ios::out);
	cout << "Введите количество записей в файле: "; 
	cin >> numRecords;

	file.close();

	cout << "Введите количество процессов Sender: ";
	cin >> numSenders;

	HANDLE* senders = new HANDLE[numSenders];
	HANDLE* events = new HANDLE[numSenders];

	//мьютекс -> успешное чтение сообщения
	HANDLE mutex = CreateMutex(nullptr, FALSE, (LPWSTR)L"mutex");
	//ограничивает количество записей, чтобы избежать переполнения файла
	HANDLE semW = CreateSemaphore(nullptr, numRecords, numRecords, L"write_sem");
	//указывает количество доступных сообщений для чтения
	HANDLE semR = CreateSemaphore(nullptr, 0, numRecords, L"read_sem");

	if (!mutex || !semW || !semR) {
		cout << "Ошибка. ";
		return -1;
	}

	//Запустить заданное количество процессов Sender. Каждому процессу Sender передать через командную строку имя файла для сообщений.
	for (int i = 0; i < numSenders; ++i) {
		wstring commandL = L"Sender.exe " + nameFile + L" " + to_wstring(numRecords) + L" " + to_wstring(i);
		//создаются события для сигнализации о готовности каждого процесса
		HANDLE ev = CreateEvent(nullptr, FALSE, FALSE, (to_wstring(i) + L"ready").c_str());
		events[i] = ev;
		senders[i] = start(commandL);
	}

	//Ждать сигнал на готовность к работе от всех процессов Sender.
	WaitForMultipleObjects(numSenders, events, TRUE, INFINITE);

	int action = 1;
	while (true) {
		cout << "1 - чтение сообщения " << endl;
		cout << "0 - выход " << endl;
		cin >> action;
		if (action != 0 && action != 1) {
			cout << "Неверная команда."<< endl;
			continue;
		}
		if (action == 0) {
			break;
		}

		//семафон не станет доступным -> ждём пока появится новое сообщение, чтобы его прочитать
		WaitForSingleObject(semR, INFINITE);
		//захватывает мьютекс, чтобы предотвратить доступ других процессов к файлу во время чтения
		WaitForSingleObject(mutex, INFINITE);

		file.open(nameFile, ios::binary | ios::in);
		message mes;
		file >> mes;
		cout << "Новое сообщение: " << mes.get() << endl;

		vector<message> text;
		while (file >> mes) {
			text.push_back(mes);
		}
		file.close();

		file.open(nameFile, ios::binary | ios::out);
		for (int i = 0; i < text.size(); ++i) {
			file << text[i];
		}
		file.close();

		//Освобождаем мьютекс
		ReleaseMutex(mutex);
		//Увеличиваем семафор, сигнализируя, что есть место для новой записи
		ReleaseSemaphore(semW, 1, nullptr);
	}

	for (int i = 0; i < numSenders; ++i) {
		CloseHandle(events[i]);
		CloseHandle(senders[i]);
	}

	delete[] events;
	delete[] senders;
	CloseHandle(mutex);
	CloseHandle(semR);
	CloseHandle(semW);

	return 0;
}
