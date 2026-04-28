// Заголовковий файл для патерну "Команда"
// Містить абстрактний клас Command та похідні команди: Transfer, Deposit, Withdraw
#ifndef COMMAND_H
#define COMMAND_H

#include "Transaction.h"

using namespace std;

// Базовий клас для команд
class Command {
public:
    virtual ~Command() = default;
    virtual Transaction* execute() = 0;
};

// Команда для переказу коштів
class TransferCommand : public Command {
private:
    string sourceAccount;  // Рахунок відправника
    string targetAccount;  // Рахунок отримувача
    double amount;        // Сума переказу

public:
    TransferCommand(const string& from, const string& to, double amt)
        : sourceAccount(from), targetAccount(to), amount(amt) {}
    
    Transaction* execute() override;

    string getTargetAccount() const { return targetAccount; }
};

// Команда для внесення коштів
class DepositCommand : public Command {
private:
    string accountId;  // Рахунок для внесення
    double amount;    // Сума внесення

public:
    DepositCommand(const string& account, double amt)
        : accountId(account), amount(amt) {}
    
    Transaction* execute() override;
};

// Команда для зняття коштів
class WithdrawCommand : public Command {
private:
    string accountId;  // Рахунок для зняття
    double amount;    // Сума зняття

public:
    WithdrawCommand(const string& account, double amt)
        : accountId(account), amount(amt) {}
    
    Transaction* execute() override;
};

#endif 