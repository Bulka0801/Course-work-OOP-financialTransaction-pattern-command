// Файл з ANSI-кодами для кольорового виводу тексту в терміналі.
#ifndef COLORS_H
#define COLORS_H

// Скидання стилю
#define RESET "\033[0m"

// Основні кольори тексту
#define WHITE "\033[37m" // Пункти меню
#define LIGHT_CYAN "\033[96m" // Заголовок меню
#define LIGHT_BLUE "\033[94m" // Межі меню
#define RED "\033[31m" // Помилки

// Фонові кольори для меню
#define BG_SOFT_BLUE "\033[48;5;153m" // Фон для заголовку меню
#define BG_SOFT_GRAY "\033[48;5;250m" // Фон для пунктів меню

// Стилі тексту
#define BOLD "\033[1m"


#endif
