// Заголовочний файл для класу Transaction — структура для зберігання даних про одну транзакцію.
#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>

using namespace std;

class Transaction {
private:
    string operationType;    // Тип операції
    double amount;          // Сума
    string accountId;       // Ідентифікатор рахунку
    time_t timestamp;       // Часова мітка
    string description;     // Опис транзакції

public:
    Transaction(const string& type, double amt, 
            const string& account, const string& desc, time_t time);   
    
    // Геттери
    string getOperationType() const { return operationType; }
    double getAmount() const { return amount; }
    string getAccountId() const { return accountId; }
    time_t getTimestamp() const { return timestamp; }
    string getDescription() const { return description; }
    
    // Форматує дату і час у читабельний формат
    string getFormattedDateTime() const {
        char buffer[26];
        struct tm* timeinfo = localtime(&timestamp);
        strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", timeinfo);
        return string(buffer);
    }
};

#endif 