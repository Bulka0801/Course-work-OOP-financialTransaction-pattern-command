#include "Transaction.h"
// Реалізація методів класу Transaction, оголошених у Transaction.h
using namespace std;
Transaction::Transaction(const string& type, double amt, 
                         const string& account, const string& desc, time_t time)
    : operationType(type)
    , amount(amt)
    , accountId(account)
    , description(desc)
    , timestamp(time) {}