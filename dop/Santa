#include <iostream>
#include <windows.h>
#include <vector>
#include <chrono>
#include <cstdlib>
using namespace std;

const int elves_num = 9;
const int reindeer_num = 9;

class SantaClaus {
private:
    HANDLE mutex;
    HANDLE elves_Sem;
    HANDLE reindeer_Sem;
    int elves_Wait; //ожидающие олени
    int reindeer_Wait; //ожидающие эльфы

public:
    SantaClaus() {
        mutex = CreateMutex(NULL, FALSE, NULL);
        elves_Sem = CreateSemaphore(NULL, 0, elves_num, NULL);
        reindeer_Sem = CreateSemaphore(NULL, 0, reindeer_num, NULL);
        elves_Wait = 0;
        reindeer_Wait = 0;
    }

    ~SantaClaus() {
        CloseHandle(mutex);
        CloseHandle(elves_Sem);
        CloseHandle(reindeer_Sem);
    }

    void Santa() {
        //блокируем доступ к  и reindeer_Wait
        WaitForSingleObject(mutex, INFINITE);

        if (reindeer_Wait >= 9) {
            cout << "Санта Клаус: Я просыпаюсь, чтобы запрячь оленей!" << endl;
            reindeer_Wait = 0; // сбрасываем счетчик оленей
            ReleaseSemaphore(reindeer_Sem, 9, NULL); // пробуждаем всех оленей = сигнализирует всем 9 оленям, что они могут продолжить свою работу
        }
        else if (elves_Wait >= 3) {
            cout << "Санта Клаус: Я просыпаюсь для совещания с эльфами!" << endl;
            elves_Wait -= 3; // сбрасываем счетчик эльфов
            ReleaseSemaphore(elves_Sem, 3, NULL); // пробуждаем группу эльфов = сигнализирует группе из 3 эльфов, что они могут продолжить свою работу
        }

        ReleaseMutex(mutex);
    }

    void elf() {
        WaitForSingleObject(mutex, INFINITE);
        elves_Wait++;
        cout << "Эльф пришел. Всего эльфов: " << elves_Wait << endl;

        if (elves_Wait == 3) {
            Santa();
        }

        ReleaseMutex(mutex);
        WaitForSingleObject(elves_Sem, INFINITE); // ждем, пока Санта не освободит эльфов
        cout << "Эльф уходит после совещания." << endl;
    }

    void reindeer() {
        WaitForSingleObject(mutex, INFINITE);
        reindeer_Wait++;
        cout << "Олень пришел. Всего оленей: " << reindeer_Wait << endl;

        if (reindeer_Wait == 9) {
            Santa();
        }

        ReleaseMutex(mutex);
        WaitForSingleObject(reindeer_Sem, INFINITE); // ждем, пока Санта не освободит оленей
        cout << "Олень уходит на свободный выпас." << endl;
    }
};

//функции реализуют отдельные потоки для эльфов и оленей, которые взаимодействуют с объектом SantaClaus
DWORD WINAPI elfThread(LPVOID param) {
    SantaClaus* santa = static_cast<SantaClaus*>(param);
    //Цикл будет выполняться бесконечно, что позволяет эльфам постоянно работать и вызывать методы Санты
    while (true) {
        Sleep(rand() % 1000 + 500); // эльф работает
        santa->elf();
    }
    return 0;
}

DWORD WINAPI reindeerThread(LPVOID param) {
    SantaClaus* santa = static_cast<SantaClaus*>(param);
    while (true) {
        Sleep(rand() % 2000 + 1000); // олень на свободном выпасе
        santa->reindeer();
    }
    return 0;
}

int main() {
    setlocale(LC_ALL, "RUS");
    srand(static_cast<unsigned>(time(0)));

    SantaClaus santa;

    vector<HANDLE> elves;
    vector<HANDLE> reindeers;

    for (int i = 0; i < elves_num; i++) {
        HANDLE elfHandle = CreateThread(NULL, 0, elfThread, &santa, 0, NULL);
        if (elfHandle != NULL) {
            elves.push_back(elfHandle);
        }
    }

    for (int i = 0; i < reindeer_num; i++) {
        HANDLE reindeerHandle = CreateThread(NULL, 0, reindeerThread, &santa, 0, NULL);
        if (reindeerHandle != NULL) {
            reindeers.push_back(reindeerHandle);
        }
    }

    // Ждем завершения потоков 
    WaitForMultipleObjects(elves.size(), elves.data(), TRUE, INFINITE);
    WaitForMultipleObjects(reindeers.size(), reindeers.data(), TRUE, INFINITE);

    // Закрываем дескрипторы потоков
    for (HANDLE elf : elves) {
        CloseHandle(elf);
    }

    for (HANDLE reindeer : reindeers) {
        CloseHandle(reindeer);
    }

    return 0;
}
