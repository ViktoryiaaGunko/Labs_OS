#include <iostream>
#include "windows.h"
using namespace std;

// массив
int* A;
int n;
int number_marker; //кол-во потоков
// структура для синхронизации доступа к критическим секциям кода
//предотвращает одновременный доступ к общим данным из разных потоков
CRITICAL_SECTION critical_s;
// указатели на массивы объектов события
HANDLE* stop_ev;
HANDLE* continue_ev;
// состояние (работает или нет) каждого потока
bool* work;


DWORD WINAPI marker(LPVOID lp) {
    auto number = (int)lp; //номер потока
    srand(number);

    //массив marked_elem будет отслеживать какие элементы массива были помечены текущим потоком
    bool* marked_elem = new bool[n];
    for (int i = 0; i < n; i++) {
        marked_elem[i] = false;
    }

    //пока поток активен
    int marked = 0;
    while (true) {
        marked = 0;
        while (true) {
            int r = rand() % n;
            EnterCriticalSection(&critical_s);
            //если элемент массива, индекс которого равен результату деления, равен нулю
            if (A[r] == 0) {
                Sleep(5);
                //заносим порядковый номер
                A[r] = number + 1;
                marked++;
                //отмечаем элемент как посещённый
                marked_elem[r] = true;
                Sleep(5);
            }
            //освобождаем критическую секцию
            LeaveCriticalSection(&critical_s);

            //В противном случае:
            if (A[r] != 0) {
                //захватываем критическую секцию
                EnterCriticalSection(&critical_s);
                //выводим инф-ию о текущем потоке
                cout << "Номер потока: " << number + 1;
                cout << "Количество помеченных элементов:" << marked;
                cout << "Индекс элемента массива, который не может быть помечен: " << r + 1;
                cout << endl;
                //устанавливаем событие остановки для текущего потока
                //освобождаем критическую секцию
                SetEvent(stop_ev[number]);
                LeaveCriticalSection(&critical_s);
                break;
            }
        }
        
        //сигнал на продолжение работы = сброс событий и продолжение работы цикла
        WaitForSingleObject(continue_ev[number], INFINITE);
        ResetEvent(continue_ev[number]);

        //сигнал на завершение работы
        if (!work[number]) {
            EnterCriticalSection(&critical_s);
            cout << number + 1 << "Закрытие.";
            //заполняем нулями в массиве все элементы, которые он пометил
            for (int i = 0; i < n; i++) {
                if (marked_elem[i]) {
                    A[i] = 0;
                }
            }
            //устанавливаем событие остановки для текущего потока
            SetEvent(stop_ev[number]);
            LeaveCriticalSection(&critical_s);
            return 0;
        }
    }
}

int main() {
    setlocale(LC_ALL, "RU");
    //1-5
    cout << "Введите количество элементов массива: ";
    cin >> n;
    A = new int[n];
    for (int i = 0; i < n; i++) {
        A[i] = 0;
    }
    cout << "Введите количество потоков marker:";
    cin >> number_marker;

    InitializeCriticalSection(&critical_s); //иниц-ия критической секции

    HANDLE* threads = new HANDLE[number_marker]; //массив дискрипторов для управления потоками
    stop_ev = new HANDLE[number_marker];  
    continue_ev = new HANDLE[number_marker]; 
    work = new bool[number_marker]; 

    //иниц-ем и создаём потоки, создаём события, запускаем каждый поток
    for (int i = 0; i < number_marker; i++) {
        stop_ev[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        continue_ev[i] = CreateEvent(NULL, TRUE, FALSE, NULL);

        threads[i] = CreateThread(NULL, 0, marker, (void*)i, 0, NULL);
        work[i] = true;
    }


    //6
    for (int i = 0; i < number_marker; i++) {
        for (int j = 0; j < number_marker; j++) {
            if (work[j])  {
                //сброс событий остановки = не мешаем работе потока, если он активен
                ResetEvent(stop_ev[j]);
            }
        }
        //ждём пока все события массива не станут сигнальными, чтобы продолжить работу
        WaitForMultipleObjects(number_marker, stop_ev, TRUE, INFINITE);

        //вывод массива
        cout << "Массив: ";
        for (int k = 0; k < n; k++) {
            cout << A[k] << " ";
        }
        cout << endl;

        int stop_thr;
        while (true) {
            cout << "Введите номер потока для остановки. ";
            cin >> stop_thr;
            stop_thr--;

            if (stop_thr >= 0 && stop_thr < number_marker && work[stop_thr]) {
                //сигнал для завершения работы потока, номер которого запросили
                work[stop_thr] = FALSE;
                //избегаем немедленной остановки
                ResetEvent(stop_ev[stop_thr]);
                //событие продолжения даёт понять, что поток может завершить своё выполнение
                SetEvent(continue_ev[stop_thr]);
                //ждём завершение потока
                WaitForSingleObject(stop_ev[stop_thr], INFINITE);
                break;
            }
            else {
                cout << "Неверный номер.";
            }
        }
        //для всех работающих потоков подаём сигнал для продолжения работы 
        for (int j = 0; j < number_marker; j++) {
            if (work[j]) {
                SetEvent(continue_ev[j]);
            }
        }
    }

    //закрываем дескрипторы для потоков и событий и освобождаем ресурсы
    for (int i = 0; i < number_marker; i++) {
        CloseHandle(threads[i]);
        CloseHandle(stop_ev[i]);
        CloseHandle(continue_ev[i]);
    }
    delete[] A;
    delete[] threads;
    delete[] stop_ev;
    delete[] continue_ev;
    delete[] work;
    //удаление критической секции
    DeleteCriticalSection(&critical_s);

    return 0;
}