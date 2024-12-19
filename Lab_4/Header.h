#include <iostream>
#include <fstream>
#include <string>
using namespace std;

struct message {
    char text[20];

    message() {
        text[0] = '\0'; 
    }

    message(const string& s) {
        if (s.length() >= sizeof(text)) {
            cout << "Ошибка: превышена максимальная длина." << endl;
            s.copy(text, sizeof(text) - 1); // Копируем строку с ограничением
            text[sizeof(text) - 1] = '\0';   
        }
        else {
            strcpy_s(text, s.c_str()); 
        }
    }

    const char* get() const {
        return text;
    }

    friend ostream& operator<<(ostream& out, const message& m) {
        // Записываем текст
        out.write(m.text, sizeof(m.text)); 
        return out;
    }

    friend istream& operator>>(istream& in, message& m) {
        // Читаем текст
        in.read(m.text, sizeof(m.text)); 
        m.text[sizeof(m.text) - 1] = '\0'; 
        return in;
    }
};
