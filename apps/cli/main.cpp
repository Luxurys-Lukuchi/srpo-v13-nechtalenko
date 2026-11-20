#include "designation_code.hpp"

#include <iostream>
#include <string>

// Платформозависимые заголовки изолированы в оболочке: предметная библиотека
// остается переносимой и не зависит от ОС и консоли.
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace {

// Определяет, запущена ли программа в интерактивной консоли (ввод с клавиатуры),
// а не через перенаправление файла или конвейер (pipe).
bool is_interactive_input() {
#ifdef _WIN32
    DWORD mode = 0;
    return GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &mode) != 0;
#else
    return isatty(STDIN_FILENO) != 0;
#endif
}

} // namespace

int main() {
#ifdef _WIN32
    // Автоматически переключаем кодовую страницу консоли Windows в UTF-8,
    // чтобы кириллица отображалась корректно без ручного вызова `chcp 65001`.
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif

    // Показываем подсказку только при ручном вводе, чтобы не засорять вывод в автотестах.
    if (is_interactive_input()) {
        std::cout << "Введите код обозначения (формат LL-DDDD, например AB-1234): " << std::flush;
    }

    // Консольная оболочка читает ровно одну строку и ничего не вычисляет сама.
    std::string line;
    if (!std::getline(std::cin, line)) {
        line.clear();
    }

    // Убираем хвостовой '\r' на случай чтения строк с переносами Windows (CRLF).
    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }

    // Все проверки, вычисления и формирование текста отказа выполняет библиотека.
    const auto result = srpo::v13::evaluate_code(line);
    if (!result.ok) {
        std::cout << result.error << '\n';
        return 0; // Штатное завершение при отказе по контракту задания
    }

    std::cout << "1. Буквенная часть: " << result.value.letters << '\n';
    std::cout << "2. Числовая часть: " << result.value.number << '\n';
    std::cout << "3. Строка кода: " << result.value.formatted_code << '\n';
    std::cout << "4. Следующий код: " << result.value.next_code << '\n';

    return 0;
}