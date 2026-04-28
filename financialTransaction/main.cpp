#include "TransactionManager.h"
#include "Colors.h"
#include <iostream>
#include <limits>
#include <iomanip>
// Точка входу в програму. Містить головне меню і логіку взаємодії з користувачем.
using namespace std;

// Допоміжна функція: рахує видиму ширину (без кольорів)
int visibleWidth(const std::string& text) {
    int width = 0;
    for (size_t i = 0; i < text.length(); ) {
        if (text[i] == '\033') {
            // Пропускаємо ANSI escape codes
            while (i < text.length() && text[i] != 'm') ++i;
            ++i;
        } else {
            // Юнікод-символ
            if ((text[i] & 0xC0) != 0x80) ++width;
            ++i;
        }
    }
    return width;
}


void clearInputBuffer() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

string centerText(const string& text, int width) {
    int spaces = (width - text.length()) / 2;
    if (spaces <= 0) return text;
    return string(spaces, ' ') + text + string(spaces + (width - text.length()) % 2, ' ');
}
void displayMenu() {
    const int width = 50;

    // Очищення екрану
    system("clear"); // для macOS/Linux
    cout << "\033[2J\033[H";

    // Верхня межа
    cout << LIGHT_BLUE << "+" << string(width - 2, '-') << "+" << RESET << "\n";

    // Заголовок меню
    cout << LIGHT_BLUE << "|" << RESET
         << BG_SOFT_BLUE << LIGHT_CYAN
         << setw(width - 2) << left << "  СИСТЕМА УПРАВЛІННЯ ФІНАНСОВИМИ ТРАНЗАКЦІЯМИ   "
         << RESET << LIGHT_BLUE << "|" << RESET << "\n";

    // Роздільник між заголовком і пунктами меню
    cout << LIGHT_BLUE << "+" << string(width - 2, '-') << "+" << RESET << "\n";

    // Пункти меню
    vector<string> options = {
        "Відкрити депозит",
        "Зняти кошти",
        "Переказати кошти",
        "Шукати за типом транзакції",
        "Шукати за рахунком",
        "Шукати за сумою",
        "Вивести всі транзакції",
        "Вийти"
    };

    for (size_t i = 0; i < options.size(); ++i) {
    string num = (i == options.size()-1) ? "0" : to_string(i+1);
    string label = num + ". " + options[i];

    // Рахуємо скільки пробілів потрібно, щоб вирівняти рядок
    int contentWidth = visibleWidth(label);
    int padding = width - 2 - contentWidth; // -2 для рамок

    // Виводимо рядок з кольорами, не порушуючи вирівнювання
    cout << LIGHT_BLUE << "|" << RESET
         << BG_SOFT_GRAY
         << LIGHT_CYAN << num << ". " << RESET
         << WHITE << options[i]
         << string(padding, ' ') << RESET
         << LIGHT_BLUE << "|" << RESET << "\n";
    }

    // Нижня межа
    cout << LIGHT_BLUE << "+" << string(width - 2, '-') << "+" << RESET << "\n";

    // Введення
    cout << BOLD << "Оберіть опцію: " << RESET;
}


void handleDeposit(TransactionManager& manager) {
    string accountId;
    double amount;
    
    cout << LIGHT_CYAN << "\n=== Зарахування коштів ===\n" << RESET;
    cout << "Введіть номер рахунку: " << RESET;
    cin >> accountId;
    
    if (manager.accountExists(accountId)) {
        cout << "Поточний баланс: " << manager.getAccountBalance(accountId) << " грн " << RESET << "\n";
    }
    
    cout << "Введіть суму для внесення: " << RESET;
    cin >> amount;
    if (!manager.checkAmount(amount)) {
        clearInputBuffer();
        return;
    }
    manager.executeCommand(new DepositCommand(accountId, amount));
    clearInputBuffer();
}

void handleWithdrawal(TransactionManager& manager) {
    string accountId;
    double amount;
    
    cout << LIGHT_CYAN << "\n=== Зняття коштів ===\n" << RESET;
    cout << "Введіть номер рахунку: " << RESET;
    cin >> accountId;
    
    if (manager.accountExists(accountId)) {
        cout << "Поточний баланс: " << manager.getAccountBalance(accountId) << " грн " << RESET << "\n";
    }
    else {
        cout << RED << BOLD << "\n[Помилка]" << RESET << RED << " Рахунок не знайдено!\n" << RESET << endl;
        clearInputBuffer();
        return;
    }
    
    cout << "Введіть суму для зняття: " << RESET;
    cin >> amount;
    if (!manager.checkAmount(amount)) {
        clearInputBuffer();
        return;
    }
    if (manager.executeCommand(new WithdrawCommand(accountId, amount))) {
        cout << "Поточний баланс: " << manager.getAccountBalance(accountId) << " грн " << RESET << "\n";
    } 
    clearInputBuffer();
}

void handleTransfer(TransactionManager& manager) {
    string sourceAccount, targetAccount;
    double amount;
    
    cout << LIGHT_CYAN << "\n=== Переказ коштів ===\n" << RESET;
    cout << "Введіть номер рахунку відправника: " << RESET;
    cin >> sourceAccount;
    
    if (!manager.accountExists(sourceAccount)) {
        cout << RED << BOLD << "\n[Помилка]" << RESET << RED << " Рахунок відправника не знайдено.\n" << RESET;
        cout << "Бажаєте створити його? (y/n): ";
        char choice;
        cin >> choice;
        if (choice == 'y' || choice == 'Y') {
            manager.createAccountIfNotExists(sourceAccount);
            cout << "Бажаєте внести кошти на рахунок \"" << sourceAccount << "\"? (y/n): ";
            char depositChoiceSrc;
            cin >> depositChoiceSrc;
            if (depositChoiceSrc == 'y' || depositChoiceSrc == 'Y') {
                double initialAmount;
                cout << "Введіть суму для внесення: ";
                cin >> initialAmount;
                if (manager.checkAmount(initialAmount)) {
                    manager.executeCommand(new DepositCommand(sourceAccount, initialAmount));
                    cout << "Рахунок поповнено. Поточний баланс: "
                         << manager.getAccountBalance(sourceAccount) << " грн\n";
                } else {
                    clearInputBuffer();
                    return;
                }
            }
        } else {
            clearInputBuffer();
            return;
        }
    } else {
        cout << "Поточний баланс: " << manager.getAccountBalance(sourceAccount) << " грн " << RESET << "\n";
    }
    
    cout << "Введіть номер рахунку отримувача: " << RESET;
    cin >> targetAccount;

    if (!manager.accountExists(targetAccount)) {
        cout << RED << BOLD << "\n[Помилка]" << RESET << RED << " Рахунок отримувача не знайдено.\n" << RESET;
        cout << "Бажаєте створити його? (y/n): ";
        char choice;
        cin >> choice;
        if (choice == 'y' || choice == 'Y') {
            manager.createAccountIfNotExists(targetAccount);
            cout << "Бажаєте внести кошти на рахунок \"" << targetAccount << "\"? (y/n): ";
            char depositChoiceTgt;
            cin >> depositChoiceTgt;
            if (depositChoiceTgt == 'y' || depositChoiceTgt == 'Y') {
                double initialAmount;
                cout << "Введіть суму для внесення: ";
                cin >> initialAmount;
                    if (manager.checkAmount(initialAmount)) {
                        manager.executeCommand(new DepositCommand(targetAccount, initialAmount));
                        cout << "Рахунок поповнено. Поточний баланс: "
                             << manager.getAccountBalance(targetAccount) << " грн\n";
                    } else {
                        clearInputBuffer();
                        return;
                    }
            }
        } else {
            clearInputBuffer();
            return;
        }
    }

    // Додати перевірку на переказ на той самий рахунок
    if (sourceAccount == targetAccount) {
        cout << RED << BOLD << "\n[Помилка]" << RESET << RED << " Неможливо переказати кошти на той самий рахунок!\n" << RESET << endl;
        clearInputBuffer();
        return;
    }
    
    cout << "Введіть суму для переказу: " << RESET;
    cin >> amount;
    if (!manager.checkAmount(amount)) {
        clearInputBuffer();
        return;
    }
    if (manager.executeCommand(new TransferCommand(sourceAccount, targetAccount, amount))) {
        cout << "Поточний баланс відправника: " << manager.getAccountBalance(sourceAccount) << " грн "<< RESET << "\n";
        cout << "Поточний баланс отримувача: " << manager.getAccountBalance(targetAccount) << " грн "<< RESET << "\n";
    } else {
        cout << RED << BOLD << "\n[Помилка]" << RESET << RED << " Недостатньо коштів на рахунку!\n" << RESET << endl;
    }
    clearInputBuffer();
}

void handleSearchByType(TransactionManager& manager) {
    cout << LIGHT_CYAN << "\n=== Пошук за типом транзакції ===\n" << RESET;
    cout << "1. Переказ коштів\n"
         << "2. Внесення коштів\n"
         << "3. Зняття коштів\n"
         << "Ваш вибір: ";
    
    int choice;
    cin >> choice;
    
    string type;
    switch(choice) {
        case 1:
            type = "TRANSFER";
            break;
        case 2:
            type = "DEPOSIT";
            break;
        case 3:
            type = "WITHDRAW";
            break;
        default:
            cout << RED << "\nПомилка: Некоректний вибір\n" << RESET;
            clearInputBuffer();
            return;
    }
    
    auto results = manager.searchByType(type);
    clearInputBuffer();
}

void handleSearchByAccount(TransactionManager& manager) {
    string accountId;
    cout << LIGHT_CYAN << "\n=== Пошук за номером рахунку ===\n" << RESET;
    cout << "Введіть номер рахунку: " << RESET;
    cin >> accountId;
    
    auto results = manager.searchByAccount(accountId);
    clearInputBuffer();
}

void handleSearchByAmount(TransactionManager& manager) {
    double minAmount, maxAmount;
    cout << LIGHT_CYAN << "\n=== Пошук за діапазоном суми ===\n" << RESET;
    cout << "Введіть мінімальну суму: " << RESET;
    cin >> minAmount;
    if (!manager.checkAmount(minAmount)) {
        clearInputBuffer();
        return;
    }
    cout << "Введіть максимальну суму: " << RESET;
    cin >> maxAmount;
    if (!manager.checkAmount(maxAmount)) {
        clearInputBuffer();
        return;
    }
    auto results = manager.searchByAmount(minAmount, maxAmount);
    clearInputBuffer();
}

int main() {
    system("chcp 65001"); // Для підтримки Unicode 
    TransactionManager manager("transactions.txt");
    int choice;
    
    do {
        displayMenu();
        cin >> choice;
        clearInputBuffer();
        
        switch (choice) {
            case 1: handleDeposit(manager); break;
            case 2: handleWithdrawal(manager); break;
            case 3: handleTransfer(manager); break;
            case 4: handleSearchByType(manager); break;
            case 5: handleSearchByAccount(manager); break;
            case 6: handleSearchByAmount(manager); break;
            case 7: manager.printTransactions(); break;
            case 0: 
                cout << "\nДякуємо за використання програми!\n" << RESET;
                break;
            default:
                cout << RED << BOLD << "\n[Помилка]" << RESET << RED << " Невірний вибір! Будь ласка, спробуйте ще раз.\n" << RESET << endl;
        }
        
        if (choice != 0) {
            cout << "\nНатисніть Enter для продовження..." << RESET;
            cin.get();
        }
        
    } while (choice != 0);
    
    return 0;
}