#include <iostream>
#include <thread>
#include <vector>
#include <windows.h>
#include <mutex>
#include <chrono>
using namespace std;

// счета
class Account {
private:

    HANDLE mutexHandle; //дескриптор мьютекса для управления доступом к счету
    long long balance;

public:

    Account(long long bal) : balance(bal) {
        mutexHandle = CreateMutex(NULL, FALSE, NULL); // 1- доступ к мьютексу имеют все пользователи, 2- нач владелец мьютекса, 3- имя 
    }

    // перевод средств
    void Transfer(Account& acc, long long sum, int first, int second) {
        // массив хранит два дескриптора мьютексов
        //mutexHandle _ текущий счёт
        //acc.mutexHandle - счёт на который переводим
        HANDLE handles[2] = { mutexHandle, acc.mutexHandle };

        // ждём пока оба мьютекса не станут свободными
        WaitForMultipleObjects(2, handles, TRUE, INFINITE);

        if (balance >= sum) {
            balance -= sum;
            acc.balance += sum;
            cout << "Переведено: " << sum << " со счета " << first + 1 << " на счет " << second + 1 << "." << endl;
            cout << "Новый баланс счета " << first + 1 << ": " << balance << endl;
            cout << "Новый баланс счета " << second + 1 << ": " << acc.balance << endl;
        }
        else {
            cout << "Перевод " << sum << " со счета " << first + 1 << " не удался. Недостаточно средств. Текущий баланс: " << balance << endl;
        }
        // освобождаем мьютексы
        ReleaseMutex(mutexHandle);
        ReleaseMutex(acc.mutexHandle);
    }


    // пополнение счёта
    void Deposit(long long sum) {
        WaitForSingleObject(mutexHandle, INFINITE);
        balance += sum;
        cout << "Пополнено: " << sum << ", Новый баланс: " << balance << endl;
        ReleaseMutex(mutexHandle);
    }

    // снятие средств со счёта
    void Withdraw(long long sum) {
        WaitForSingleObject(mutexHandle, INFINITE);
        if (balance >= sum) {
            balance -= sum;
            cout << "Снято: " << sum << ", Новый баланс: " << balance << endl;
        }
        else {
            cout << "Снятие " << sum << " не удалось. Недостаточно средств. Текущий баланс: " << balance << endl;
        }
        ReleaseMutex(mutexHandle);
    }

    // получение счёта
    long long Get_Balance() {
        WaitForSingleObject(mutexHandle, INFINITE);
        long long bal = balance;
        ReleaseMutex(mutexHandle);
        return bal;
    }

};

// выполнение операций со счетом
void Operations(vector<Account>& accounts, int id) {
    int op;
    long long sum;
    int a = -1;

    cout << "Запрос " << (id + 1) << ": Выберите действие (1: пополнение, 2: снятие, 3: перевод): ";
    cin >> op;

    // Запрос номера счета для пополнения или снятия
    if (op == 1 || op == 2) {
        cout << "Введите номер счета (1-" << accounts.size() << "): ";
        cin >> a;
        a--; // Преобразуем в 0-индекс
    }

    cout << "Введите сумму: ";
    cin >> sum;

    // Обработка перевода между счетами
    if (op == 3) {
        int first = -1, second = -1;
        cout << "Введите номер счета отправителя (1-" << accounts.size() << "): ";
        cin >> first;
        first--;

        cout << "Введите номер счета получателя (1-" << accounts.size() << "): ";
        cin >> second;
        second--;

        if (first >= 0 && first < accounts.size() && second >= 0 && second < accounts.size()) {
            accounts[first].Transfer(accounts[second], sum, first, second);
        }
        else {
            cout << "Некорректные номера счетов." << endl;
        }
    }
    else if (op == 1 && a >= 0 && a < accounts.size()) {
        accounts[a].Deposit(sum);
    }
    else if (op == 2 && a >= 0 && a < accounts.size()) {
        accounts[a].Withdraw(sum);
    }
    else {
        cout << "Запрос " << (id + 1) << ": Некорректное действие." << endl;
    }
}

int main() {
    setlocale(LC_ALL, "RU");

    const int num_accounts = 5; //счета
    int num_threads; //потоки

    vector<Account> accounts;
    vector<thread> threads; // управление потоками

    // ввод начальных балансов
    for (int i = 0; i < num_accounts; ++i) {
        long long bal;
        cout << "Введите начальный баланс для счета " << (i + 1) << ": ";
        cin >> bal;
        accounts.emplace_back(bal);
    }

    cout << "Введите количество потоков: ";
    cin >> num_threads;

    for (int i = 0; i < num_threads; ++i) {
        // добавления нового потока в вектор threads
        // Operations будет выполняться в новом потоке
        // ref(accounts) ссылка на вектор accounts чтобы не копировать весь вектор
        // i — идентификатор потока, чтобы поток знал, какой номер ему присвоен
        threads.emplace_back(Operations, ref(accounts), i);
        // ждем, пока текущий поток Operations завершит свою работу, прежде чем продолжить выполнение следующей итерации цикла
        threads.back().join(); // ждем завершения потока перед переходом к следующему
        this_thread::sleep_for(chrono::seconds(1));
    }

    cout << endl << "Финальные балансы: " << endl ;
    for (int i = 0; i < num_accounts; ++i) {
        cout << "Счет " << (i + 1) << ": " << accounts[i].Get_Balance() << endl;
    }
    return 0;
}