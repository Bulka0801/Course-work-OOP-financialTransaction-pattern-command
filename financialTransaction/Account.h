// Заголовочний файл для класу Account, який описує об'єкт користувача з рахунком.
#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <string>
#include <map>                                              

using namespace std;

class Account {
private:
    string accountId;  // Ідентифікатор рахунку
    double balance;    // Баланс рахунку

public:
    Account() : accountId(""), balance(0.0) {}
    
    Account(const string& id, double initialBalance = 0.0) 
        : accountId(id), balance(initialBalance) {}

    // Знімає кошти з рахунку
    bool withdraw(double amount) {
        if (amount > balance) return false;
        balance -= amount;
        return true;
    }

    // Поповнює рахунок
    void deposit(double amount) {
        balance += amount;
    }

    // Геттери
    double getBalance() const { return balance; }
    string getId() const { return accountId; }
};

#endif 