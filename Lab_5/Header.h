#pragma once
#include <iostream>

struct employee {
    int num;           // ID 
    char name[10];     
    double hours;      

    employee() : num(0), hours(0) {
        strcpy_s(name, "Unnamed"); 
    }

    employee(const employee& emp) {
        num = emp.num; 
        hours = emp.hours; 
        strcpy_s(name, emp.name); 
    }
};
