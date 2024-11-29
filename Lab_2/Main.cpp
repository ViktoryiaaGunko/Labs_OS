#define _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING
#include <iostream>
#include <vector>
#include <windows.h>
#include <thread>
#include <mutex>
#include <gtest/gtest.h>
using namespace std;

//структура для данных потоков
struct Data {
    vector<int> numbs;
    int min;
    int max;
    double average = 0.0;
};

//ф-ия для потока Min_Max
DWORD WINAPI Min_Max(LPVOID lp) {

    Data* a = (Data*)lp; //приведение lp к указателю на Data
    size_t n = a->numbs.size();
    
    if (n == 0) return 0;

    a->min = a->numbs[0];
    a->max = a->numbs[0];

    //поиск min и max элементов массива
    for (int i = 0; i < n; ++i) {
        if (a->numbs[i] < a->min) {
            a->min = a->numbs[i];
        }
        Sleep(7);
        if (a->numbs[i] > a->max) {
            a->max = a->numbs[i];
        }
        Sleep(7);
    }
    cout << "Minimum element: " << a->min << endl;
    cout << "Maximum element: " << a->max << endl;
    return 0;
}

//ф-ия для потока average
DWORD WINAPI Average(LPVOID lp) {

    Data* a = (Data*)lp;//приведение lp к указателю на Data
    size_t n = a->numbs.size();

    int sum = 0;
    for (int i = 0; i < n; ++i) {
        sum += a-> numbs[i] ;
        Sleep(12);
    }
    a->average = static_cast<double>(sum) / n;
    cout << "Average value: " << a->average << endl;
    return 0;
}


int main() {

    cout << "Enter the size of the array: ";
    int size;
    cin >> size;

    vector<int> numbs_(size);
    cout << "Enter the elements of the array: ";
    for (int i = 0; i < size; ++i) {
        cin >> numbs_[i] ;
    }

    Data a = { numbs_ };
    //переменные для хранения дискр-ов потоков
    HANDLE hMin_Max = CreateThread(nullptr, 0, Min_Max, &a, 0, nullptr); //0 - размер стека по умочанию
    HANDLE hAverage = CreateThread(nullptr, 0, Average, &a, 0, nullptr);

    WaitForSingleObject(hMin_Max, INFINITE);  
    WaitForSingleObject(hAverage, INFINITE); 
    //INFINITE -> ожидание будет пока поток не завершится.

    //замена
    for (int i = 0; i < numbs_.size(); ++i) { 
        if (numbs_[i] == a.min || numbs_[i] == a.max) { 
            numbs_[i] = static_cast<int>(a.average); 
        }
    }

    cout << "New array: ";
    for (int i = 0; i < numbs_.size(); ++i) {
        cout << numbs_[i] << " "; 
    }

    CloseHandle(hMin_Max);
    CloseHandle(hAverage);

    return 0;
}
