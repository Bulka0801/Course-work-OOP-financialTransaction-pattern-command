// Реалізація TransactionManager — додавання, фільтрація, сортування та вивід транзакцій.
#include "TransactionManager.h"
#include "Colors.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <ctime>
#include <termios.h>
#include <unistd.h>
#include <cstdio>

// Функція для зчитування одного символу без натискання Enter
char getch() {
    char buf = 0; // Змінна для збереження натиснутої клавіші

    struct termios old = {}; // Структура для збереження старих налаштувань терміналу

    // Отримуємо поточні налаштування терміналу (Terminal Control Get Attributes)
    if (tcgetattr(STDIN_FILENO, &old) < 0)
        perror("tcsetattr()"); // Виводимо помилку, якщо не вдалося отримати налаштування

    // Вимикаємо канонічний режим і відображення введення
    old.c_lflag &= ~ICANON; // Вимикаємо канонічний режим (не чекати Enter)
    old.c_lflag &= ~ECHO;   // Вимикаємо відображення введених символів

    // Застосовуємо змінені налаштування одразу (Terminal Control Set Attributes)
    if (tcsetattr(STDIN_FILENO, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON"); // Помилка при встановленні нових налаштувань

    // Зчитуємо один символ із стандартного вводу
    if (read(STDIN_FILENO, &buf, 1) < 0)
        perror("read()"); // Помилка при читанні символу

    // Повертаємо налаштування терміналу до попередніх
    old.c_lflag |= ICANON; // Вмикаємо назад канонічний режим
    old.c_lflag |= ECHO;   // Вмикаємо назад відображення введення

    // Застосовуємо старі налаштування
    if (tcsetattr(STDIN_FILENO, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON"); // Помилка при поверненні налаштувань

    return buf; // Повертаємо натиснений символ
}

using namespace std;

// Константи для обмежень
const double MIN_AMOUNT = 0.01;           // Мінімальна сума транзакції
const double MAX_AMOUNT = 1000000.0;      // Максимальна сума транзакції
const double SUSPICIOUS_AMOUNT = 50000.0; // Сума, що потребує додаткової перевірки
const double DAILY_LIMIT = 1000000.0;     // Денний ліміт на транзакції


TransactionManager::TransactionManager(const string &filename)
    : transactionFile(filename){ loadTransactions(); }

TransactionManager::~TransactionManager() {
    saveTransactions();
    for (auto *trans : transactions){ delete trans; } 
}

// Завантажуємо транзакції з файлу
void TransactionManager::loadTransactions() {
    ifstream file(transactionFile);
    if (!file.is_open())
        return;

    string dateTimeStr, type, accountId, desc;
    double amount;
    while (getline(file, dateTimeStr)){
        // Формат: "YYYY-MM-DD HH:MM:SS TYPE AMOUNT ACCOUNT_ID DESCRIPTION"
        stringstream ss(dateTimeStr);
        string date, time;
        getline(ss, date, ' ');
        getline(ss, time, ' ');
        getline(ss, type, ' ');
        ss >> amount >> accountId;
        ss.ignore(); // пропускаємо пробіл після суми
        getline(ss, desc);

        // Перетворення текстову дату та час у формат, який розуміє комп'ютер
        struct tm tm_time = {}; // Створюємо порожню структуру для дати/часу
        stringstream datetime_stream(date + " " + time); // Об'єднуємо дату і час
        datetime_stream >> get_time(&tm_time, "%Y-%m-%d %H:%M:%S"); // Перетворюємо текст в структуру
        time_t timestamp = mktime(&tm_time); // Перетворюємо структуру в Unix-час ( в кількість секунд від 1 січня 1970 року)

        createAccountIfNotExists(accountId);

        if (type == "WITHDRAW"){
            accounts[accountId].withdraw(amount);
        }
        else if (type == "DEPOSIT"){
            accounts[accountId].deposit(amount);
        }
        else if (type == "TRANSFER"){
            size_t pos = desc.find("на рахунок "); // pos буде позицією, де починається "на рахунок"
            if (pos != string::npos){ // спеціальне значення, яке означає "не знайдено"
                string targetAccount = desc.substr(pos + 11); // Бере частину тексту після "на рахунок " (11 - це довжина фрази "на рахунок) "
                createAccountIfNotExists(targetAccount);
                accounts[accountId].withdraw(amount);
                accounts[targetAccount].deposit(amount);}
        }
        // Створюємо транзакцію з додаванням timestamp
        transactions.push_back(new Transaction(type, amount, accountId, desc, timestamp));
    }
}

// Функція для збереження транзакцій у файл
void TransactionManager::saveTransactions() {
    ofstream file(transactionFile);
    if (!file.is_open()){
        cout << RED << BOLD << "\n[Помилка]" << RESET << RED << " Не вдалося відкрити файл для збереження\n"
             << RESET << endl;
        return; }

    for (const auto *trans : transactions){
        // Форматуємо дату та час у вигляді "YYYY-MM-DD HH:MM:SS"
        string formattedDateTime = trans->getFormattedDateTime();

        file << formattedDateTime << " "
             << trans->getOperationType() << " "
             << trans->getAmount() << " "
             << trans->getAccountId() << " "
             << trans->getDescription() << "\n";
    }
    file.close();
}

bool TransactionManager::checkAmount(double amount) const {
    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << RED << BOLD << "\n[Помилка]" << RESET << RED << " Сума повинна бути числом\n"
             << RESET << endl;
        return false; }
    if (amount <= 0) {
        cout << RED << BOLD << "\n[Помилка]" << RESET << RED << " Сума має бути більше 0\n"
             << RESET << endl;
        return false; }
    if (amount < MIN_AMOUNT) {
        cout << RED << BOLD << "\n[Помилка]" << RESET << RED << " Сума має бути не менше " << MIN_AMOUNT << " грн\n" 
            << RESET << endl;
        return false; }
    if (amount > MAX_AMOUNT) {
        cout << RED << BOLD << "\n[Помилка]" << RESET << RED << " Сума не може перевищувати " << fixed << setprecision(0) << MAX_AMOUNT << " грн\n"
             << RESET << endl;
        return false; }
    return true;
}

bool TransactionManager::checkDailyLimit(const string &accountId, double amount) const {
    double dailyTotal = getDailyTransactionTotal(accountId);
    if (dailyTotal + amount > DAILY_LIMIT) {
        cout << RED << BOLD << "\n[Помилка]" << RESET << RED << " Перевищено денний ліміт операцій\n"
             << RESET << endl;
        return false; }
    return true;
}

bool TransactionManager::checkTransfer(const string &fromAccount, const string &toAccount, double amount) const {
    if (fromAccount == toAccount) {
        cout << RED << BOLD << "\n[Помилка]" << RESET << RED << " Неможливо переказати кошти на той самий рахунок\n"
             << RESET << endl;
        return false; }
    if (!canWithdraw(fromAccount, amount)) {
        cout << RED << BOLD << "\n[Помилка]" << RESET << RED << " Недостатньо коштів на рахунку\n"
             << RESET << endl;
        return false; }
    return true;
}

// Основна функція виконання команд (внесення, зняття, переказ)
// Приймає команду, виконує її, перевіряє умови, оновлює рахунки та зберігає транзакцію
bool TransactionManager::executeCommand(Command *command) {
    // Виконуємо команду (створюється об'єкт транзакції)
    Transaction *transaction = command->execute();

    // Якщо транзакція не створена — видаляємо команду і завершуємо
    if (!transaction){
        delete command;
        return false; }

    string type = transaction->getOperationType();
    string accountId = transaction->getAccountId();
    double amount = transaction->getAmount();

    // Перевірка суми
    if (!checkAmount(amount)) {
        delete transaction;
        delete command;
        return false; }

    // Перевірка на підозріло велику суму (більше 50 000 грн)
    if (amount >= SUSPICIOUS_AMOUNT) {
        cout << RED << BOLD << "\n[Помилка]" << RESET << RED << " Транзакції на суму від " << SUSPICIOUS_AMOUNT
             << " грн потребують додаткової верифікації" << RESET << endl;
        cout << "Будь ласка, зверніться до відділення банку" << RESET << endl;
        delete transaction;
        delete command;
        return false; }

    // Перевірка денного ліміту
    if (!checkDailyLimit(accountId, amount)) {
        delete transaction;
        delete command;
        return false; }

    // Створюємо рахунок, якщо не існує
    createAccountIfNotExists(accountId);

    // Визначаємо тип транзакції та виконуємо відповідну логіку
    if (type == "WITHDRAW") {
        // Перевірка балансу для зняття
        if (!canWithdraw(accountId, amount)) {
            cout << RED << BOLD << "\n[Помилка]" << RESET << RED << " Недостатньо коштів на рахунку\n"
                 << RESET << endl;
            cout << "Доступний баланс: " << getAccountBalance(accountId) << " грн" << RESET << endl;
            delete transaction;
            delete command;
            return false; }
        accounts[accountId].withdraw(amount);
        cout << "Кошти успішно знято з рахунку!" << RESET << endl;
    }
    else if (type == "DEPOSIT") {
        accounts[accountId].deposit(amount);
        cout << "Кошти успішно внесено на рахунок!" << RESET << endl;
    }
    else if (type == "TRANSFER") {
        // Перетворюємо в TransferCommand, щоб отримати targetAccount напряму
        auto *transferCmd = dynamic_cast<TransferCommand *>(command);
        if (!transferCmd) {
            cout << RED << BOLD << "\n[Помилка]" << RESET << RED << " Неможливо обробити команду переказу\n"
                 << RESET << endl;
            delete transaction;
            delete command;
            return false; }

        string targetAccount = transferCmd->getTargetAccount();

        // Перевірка існування рахунків і пропозиція створення
        if (!accountExists(accountId)) {
            cout << RED << BOLD << "\n[Помилка]" << RESET << RED << " Рахунок відправника не існує.\n" << RESET;
            cout << "Бажаєте створити його? (y/n): ";
            char choice;
            cin >> choice;
            if (choice == 'y' || choice == 'Y') {
                createAccountIfNotExists(accountId);
            } else {
                delete transaction;
                delete command;
                return false; }
        }

        if (!accountExists(targetAccount)) {
            cout << RED << BOLD << "\n[Помилка]" << RESET << RED << " Рахунок отримувача не існує.\n" << RESET;
            cout << "Бажаєте створити його? (y/n): ";
            char choice;
            cin >> choice;
            if (choice == 'y' || choice == 'Y') {
                createAccountIfNotExists(targetAccount);
            } else {
                delete transaction;
                delete command;
                return false;
            }
        }

        if (!canWithdraw(accountId, amount)) {
            cout << RED << BOLD << "\n[Помилка]" << RESET << RED << " Недостатньо коштів для переказу.\n" << RESET;
            cout << "Доступний баланс: " << getAccountBalance(accountId) << " грн" << RESET << endl;
            delete transaction;
            delete command;
            return false;
        }

        // Виконуємо переказ
        accounts[accountId].withdraw(amount);
        accounts[targetAccount].deposit(amount);

        cout << "Переказ успішно виконано!" << RESET << endl;
    }

    // Зберігаємо транзакцію та оновлюємо файл
    transactions.push_back(transaction);
    saveTransactions();

    delete command;
    return true;
}

// Додаємо нову функцію для перевірки денного ліміту транзакцій
double TransactionManager::getDailyTransactionTotal(const string &accountId) const {
    double total = 0.0;
    time_t now = time(nullptr);
    tm *today = localtime(&now);

    for (const auto *trans : transactions)
    {
        if (trans->getAccountId() == accountId)
        {
            time_t transTimestamp = trans->getTimestamp();
            tm *transTime = localtime(&transTimestamp);
            if (transTime->tm_year == today->tm_year &&
                transTime->tm_mon == today->tm_mon &&
                transTime->tm_mday == today->tm_mday)
            {
                total += trans->getAmount();
            }
        }
    }
    return total;
}

// Виводить всю історію транзакцій у вигляді таблиці
void TransactionManager::printTransactions() const
{
    // Створюємо чергу з усіх транзакцій та передаємо її у основну функцію
    printTransactions(deque<Transaction *>(transactions.begin(), transactions.end()));
}

// Виводить переданий список транзакцій у вигляді таблиці
void TransactionManager::printTransactions(const deque<Transaction *> &transactionList) const {
    if (transactionList.empty()) {
        cout << "\nІсторія транзакцій пуста.\n" << RESET;
        return; }

    const int recordsPerPage = 15;
    // Обчислюємо загальну кількість сторінок (округлення вгору для неповних сторінок)
    int totalPages = (transactionList.size() + recordsPerPage - 1) / recordsPerPage;
    int currentPage = 1;

    while (true) {
        // Очищення екрану перед виведенням нової сторінки
        system("clear"); // Для Mac/Linux 
        cout << LIGHT_CYAN << "\nТаблиця транзакцій (Сторінка " << currentPage << " із " << totalPages << "):\n" << RESET;

        string line = string(114, '-');
        cout << line << "\n";
        cout << "| " << setw(24) << left << "Дата"
             << " | " << setw(13) << left << "Тип"
             << " | " << setw(12) << left << "Сума"
             << " | " << setw(15) << left << "Рахунок"
             << " | " << setw(43) << left << "Опис"
             << " | " << setw(16) << right << "Баланс"
             << " |\n";
        cout << line << "\n";

        int startIdx = (currentPage - 1) * recordsPerPage;
        int endIdx = min(startIdx + recordsPerPage, (int)transactionList.size());

        // Виведення транзакцій для поточної сторінки
        for (int i = startIdx; i < endIdx; ++i) {
            const auto *trans = transactionList[i];
            string date = trans->getFormattedDateTime();
            string type = trans->getOperationType();
            string account = trans->getAccountId();
            string desc = trans->getDescription();

            stringstream amountStream, balanceStream;
            amountStream << fixed << setprecision(2) << trans->getAmount();
            balanceStream << fixed << setprecision(2) << getAccountBalance(account);

            string amount = amountStream.str();
            string balance = balanceStream.str();

            const int maxDateLen = 20;
            const int maxTypeLen = 10;
            const int maxAmountLen = 8;
            const int maxAccountLen = 8;
            const int maxDescLen = 39;
            const int maxBalanceLen = 10;

            // Розбиття довгого опису транзакції на декілька рядків для коректного відображення в таблиці
            vector<string> descLines;
            size_t j = 0;
            while (j < desc.length()) {
                string line;
                int charCount = 0;
                while (j < desc.length() && charCount < maxDescLen) {
                    char c = desc[j];
                    line += c;
                    if ((c & 0xC0) != 0x80)
                        ++charCount;
                    ++j;
                }
                if (charCount < maxDescLen)
                    line += string(maxDescLen - charCount, ' ');
                descLines.push_back(line);
            }

            cout << "| " << setw(maxDateLen) << left << date
                 << " | " << setw(maxTypeLen) << left << type
                 << " | " << setw(maxAmountLen) << right << amount
                 << " | " << setw(maxAccountLen) << right << account
                 << " | " << setw(maxDescLen) << left << descLines[0]
                 << " | " << setw(maxBalanceLen) << right << balance
                 << " |\n";
        }
        cout << line << "\n";

        cout << "\n[→] Наступна сторінка | [←] Попередня сторінка | [q] Вийти\n";
        cout << "Ваш вибір (стрілками або q): ";

        // Обробка натискань стрілок клавіатури для переходу між сторінками
        char c = getch();
        if (c == 27) { // Escape-символ (початок ESC-послідовності)
            if (getch() == 91) { // [
                char dir = getch();
                if (dir == 67) { // →
                    if (currentPage < totalPages) currentPage++;
                } else if (dir == 68) { // ←
                    if (currentPage > 1) currentPage--;
                }
            }
        }
        // Вихід із режиму перегляду транзакцій
        else if (c == 'q' || c == 'Q') {
            break;
        } else {
            cout << "\nНевідомий вибір, використовуйте стрілки або 'q'.\n";
        }
    }
}

// Перевіряємо, чи існує рахунок
bool TransactionManager::accountExists(const string &accountId) const {
    return accounts.find(accountId) != accounts.end();
}

// Отримуємо баланс рахунку
double TransactionManager::getAccountBalance(const string &accountId) const {
    auto iter = accounts.find(accountId);
    return iter != accounts.end() ? iter->second.getBalance() : 0.0;
}

// Створюємо рахунок, якщо його немає
void TransactionManager::createAccountIfNotExists(const string &accountId) {
    if (!accountExists(accountId)) {
        accounts.insert(make_pair(accountId, Account(accountId)));
        cout << "Створено новий рахунок: " << accountId << RESET << endl; }
}

// Перевіряємо, чи можна зняти кошти з рахунку
bool TransactionManager::canWithdraw(const string &accountId, double amount) const {
    auto iter = accounts.find(accountId);
    return iter != accounts.end() && iter->second.getBalance() >= amount;
}

// Вектор вказівників на об’єкти типу Transaction.
// Тобто функція повертає список (масив, динамічну колекцію) усіх знайдених транзакцій, які задовольняють певну умову.
// Пошук транзакцій за типом (звичайний лінійний пошук)
vector<Transaction *> TransactionManager::searchByType(const string &type) const {
    vector<Transaction *> result; //створюється порожній вектор, куди ми будемо додавати знайдені транзакції
    for (auto *trans : transactions){
        if (trans->getOperationType() == type){
            result.push_back(trans);
        }
    }
    if (!result.empty()){
        cout << LIGHT_CYAN << "\nЗнайдено " << result.size() << " транзакцій:\n"
             << RESET;
        printTransactions(deque<Transaction *>(result.begin(), result.end()));
    }
    else{
        cout << "\nТранзакції типу '" << type << "' не знайдено.\n"
             << RESET;
    }
    return result;
}

// Пошук транзакцій за номером рахунку
vector<Transaction *> TransactionManager::searchByAccount(const string &accountId) const {
    vector<Transaction *> result;
    for (auto *trans : transactions){
        if (trans->getAccountId() == accountId){
            result.push_back(trans);
        }
    }
    if (result.empty()) {
        cout << "\nТранзакції для рахунку '" << accountId << "' не знайдено.\n"
             << RESET;
    }
    else if (result.size() == 1){
        cout << LIGHT_CYAN << "\nЗнайдено 1 транзакцію:\n"
             << RESET;
        const auto *trans = result[0];
        cout << "-------------------\n"
             << "Тип: " << trans->getOperationType() << "\n"
             << "Сума: " << trans->getAmount() << " грн\n"
             << "Рахунок: " << trans->getAccountId() << "\n"
             << "Опис: " << trans->getDescription() << "\n"
             << "Дата: " << trans->getFormattedDateTime() << "\n"
             << "-------------------\n";
    }
    else{
        cout << LIGHT_CYAN << "\nЗнайдено " << result.size() << " транзакцій:\n"
             << RESET;
        printTransactions(deque<Transaction *>(result.begin(), result.end()));
    }
    return result;
}

// Пошук транзакцій за діазоном суми
vector<Transaction *> TransactionManager::searchByAmount(double minAmount, double maxAmount) const {
    vector<Transaction *> result;
    for (auto *trans : transactions) {
        double amount = trans->getAmount();
        if (amount >= minAmount && amount <= maxAmount) {
            result.push_back(trans);
        }
    }
    if (result.empty()) {
        cout << "\nТранзакції в діапазоні " << minAmount << " - " << maxAmount << " грн\n"
             << RESET;
    }
    else if (result.size() == 1) {
        cout << LIGHT_CYAN << "\nЗнайдено 1 транзакцію:\n"
             << RESET;
        const auto *trans = result[0];
        cout << "-------------------\n"
             << "Тип: " << trans->getOperationType() << "\n"
             << "Сума: " << trans->getAmount() << " грн\n"
             << "Рахунок: " << trans->getAccountId() << "\n"
             << "Опис: " << trans->getDescription() << "\n"
             << "Дата: " << trans->getFormattedDateTime() << "\n"
             << "-------------------\n";
    }
    else {
        cout << LIGHT_CYAN << "\nЗнайдено " << result.size() << " транзакцій:\n"
             << RESET;
        printTransactions(deque<Transaction *>(result.begin(), result.end()));
    }
    return result;
}