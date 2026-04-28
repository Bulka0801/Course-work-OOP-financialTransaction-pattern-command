// Реалізація команд, оголошених у Command.h
// Реалізація шаблону "Команда" — логіка виконання фінансових дій через об'єкти-команди.
#include "Command.h"
#include "TransactionManager.h"

using namespace std;

Transaction* TransferCommand::execute() {
    string description = "Переказ коштів з рахунку " + sourceAccount + " на рахунок " + targetAccount;
    return new Transaction("TRANSFER", amount, sourceAccount, description, time(nullptr));
}

Transaction* DepositCommand::execute() {
    string description = "Зарахування коштів на рахунок " + accountId;
    return new Transaction("DEPOSIT", amount, accountId, description, time(nullptr));
}

Transaction* WithdrawCommand::execute() {
    string description = "Зняття коштів з рахунку " + accountId;
    return new Transaction("WITHDRAW", amount, accountId, description, time(nullptr));
}
