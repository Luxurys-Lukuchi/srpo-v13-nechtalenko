#include "designation_code.hpp"

#include <iostream>
#include <string>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace {

// Включает кодировку UTF-8 в консоли Windows для корректного вывода кириллицы.
void configure_console_utf8() {
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif
}

// Определяет, запущена ли программа интерактивно (ввод с клавиатуры)
// или данные поданы через конвейер/перенаправление (например, "GH-0001" | .\app.exe).
bool is_interactive_stdin() {
#ifdef _WIN32
    DWORD console_mode = 0;
    return GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &console_mode) != 0;
#else
    return isatty(STDIN_FILENO) != 0;
#endif
}

}  // namespace

int main() {
    configure_console_utf8();

    // При интерактивном запуске показываем пример ввода.
    if (is_interactive_stdin()) {
        std::cout << "Введите код обозначения (формат LL-DDDD, например AB-1234): " << std::flush;
    }

    // Читаем ровно одну строку (при пустом потоке line останется пустой, и отказ сформирует библиотека).
    std::string input_line;
    std::getline(std::cin, input_line);

    // Убираем завершающий '\r' на случай файла/конвейера с Windows-переводами строк CRLF.
    if (!input_line.empty() && input_line.back() == '\r') {
        input_line.pop_back();
    }

    // Передаем запись в предметную библиотеку — вся логика и текст отказа находятся там.
    const srpo::v13::EvaluationResult result = srpo::v13::evaluate_code(input_line);

    if (!result.ok) {
        // Неверный вход отвергается явно, программа завершается штатно без печати результатов.
        std::cout << result.error << '\n';
        return 0;
    }

    // Выводим результаты всех четырех операций варианта 13.
    std::cout << "1. Буквенная часть: " << result.letters << '\n';
    std::cout << "2. Числовая часть: " << result.number << '\n';
    std::cout << "3. Строка кода: " << result.formatted_code << '\n';
    std::cout << "4. Следующий код: " << result.next_code << '\n';

    return 0;
}