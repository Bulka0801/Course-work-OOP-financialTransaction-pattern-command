#!/bin/bash

# Встановлюємо кодування UTF-8
export LANG=en_US.UTF-8
export LC_ALL=en_US.UTF-8

# Компілюємо програму
echo "Компіляція програми..."
g++ -std=c++11 -fexec-charset=UTF-8 main.cpp Transaction.cpp TransactionManager.cpp Command.cpp -o financialTransaction

# Перевіряємо результат компіляції
if [ $? -eq 0 ]; then
    echo "Компіляція успішна!"
    echo "Запуск програми..."
    ./financialTransaction
else
    echo "Помилка компіляції! Перевірте помилки вище."
fi 