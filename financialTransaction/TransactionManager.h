// Заголовочний файл для TransactionManager — клас, який керує всіма транзакціями.
#ifndef TRANSACTION_MANAGER_H
#define TRANSACTION_MANAGER_H

#include <deque>
#include <string>
#include <vector>
#include <map>
#include "Command.h"
#include "Transaction.h"
#include "Account.h"

using namespace std;

class TransactionManager {
private:
    deque<Transaction*> transactions;
    map<string, Account> accounts;
    string transactionFile;

    void loadTransactions();
    void saveTransactions();
    double getDailyTransactionTotal(const string& accountId) const;

    // Перевірки
    bool checkDailyLimit(const string& accountId, double amount) const;
    bool checkTransfer(const string& fromAccount, const string& toAccount, double amount) const;

public:
    explicit TransactionManager(const string& filename = "transactions.txt");
    ~TransactionManager();

    // Команд execution
    bool executeCommand(Command* command);
    
    // Операції по рахунку
    bool accountExists(const string& accountId) const;

    // Перевірки
    bool checkAmount(double amount) const;

    double getAccountBalance(const string& accountId) const;
    void createAccountIfNotExists(const string& accountId);
    bool canWithdraw(const string& accountId, double amount) const;
    
    // Пошук транзакцій за типом, номером рахунку та діапозоном суми
    vector<Transaction*> searchByType(const string& type) const;
    vector<Transaction*> searchByAccount(const string& accountId) const;
    vector<Transaction*> searchByAmount(double minAmount, double maxAmount) const;
    
    // Виводить всю історію транзакцій у вигляді таблиці
    // Використовується для виводу результатів пошуку
    // та для виводу всіх транзакцій 
    void printTransactions() const;
    void printTransactions(const deque<Transaction*>& transactionList) const;
};

// Обрізає текст з "..." в кінці
inline string trimText(const string& text, size_t maxLen) {
    if (text.length() > maxLen) {
        return text.substr(0, maxLen - 3) + "...";
    }
    return text;
}

// Обрізає числа з "+" в кінці
inline string trimNumber(double num, size_t maxLen) {
    string str = to_string(num);
    if (str.length() > maxLen) {
        return str.substr(0, maxLen - 1) + "+";
    }
    return str;
}

#endif 