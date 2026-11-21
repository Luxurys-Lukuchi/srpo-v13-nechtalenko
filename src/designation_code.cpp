#include "designation_code.hpp"

#include <cassert>
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

namespace srpo::v13 {

namespace {

// Явные проверки ASCII-диапазонов безопаснее std::isupper/std::isdigit:
// они не зависят от системной локали и не вызывают неопределенного поведения
// на отрицательных значениях char (например, на байтах кириллицы в UTF-8).
bool is_ascii_space(char ch) {
    return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n' || ch == '\v' || ch == '\f';
}

bool is_uppercase_latin(char ch) {
    return ch >= 'A' && ch <= 'Z';
}

bool is_ascii_digit(char ch) {
    return ch >= '0' && ch <= '9';
}

// Формирует результат отказа с готовым текстом для оболочки.
EvaluationResult make_error(std::string message) {
    EvaluationResult result;
    result.ok = false;
    result.error = std::move(message);
    return result;
}

// Внутренняя функция форматирования целого числа 0..9999 ровно в 4 цифры с ведущими нулями.
// Снаружи библиотеки недоступна: вызывается только после проверки диапазона.
std::string format_number_4(int number) {
    // Предусловие (внутренний инвариант): диапазон гарантирован вызывающим кодом.
    // В Debug страхует от ошибок программиста, в Release (-DNDEBUG) удаляется препроцессором.
    assert(number >= 0 && number <= 9999);

    std::string digits(4, '0');
    digits[0] = static_cast<char>('0' + (number / 1000) % 10);
    digits[1] = static_cast<char>('0' + (number / 100) % 10);
    digits[2] = static_cast<char>('0' + (number / 10) % 10);
    digits[3] = static_cast<char>('0' + number % 10);

    // Постусловие: ширина числовой части строго 4 символа.
    assert(digits.size() == 4);
    return digits;
}

}  // namespace

EvaluationResult evaluate_code(std::string_view raw_input) {
    // 1. Внешний контракт: проверяем вход пользователя через if (работает всегда, включая Release).
    if (raw_input.empty()) {
        return make_error("Отказ: пустой ввод (ожидается код в формате LL-DDDD, например AB-1234).");
    }

    for (char ch : raw_input) {
        if (is_ascii_space(ch)) {
            return make_error("Отказ: код не должен содержать пробелы или символы перевода строки.");
        }
    }

    if (raw_input.size() != 7) {
        return make_error("Отказ: неверная длина кода (ожидается ровно 7 символов в формате LL-DDDD).");
    }

    if (!is_uppercase_latin(raw_input[0]) || !is_uppercase_latin(raw_input[1])) {
        return make_error("Отказ: первые два символа должны быть заглавными латинскими буквами (A-Z).");
    }

    if (raw_input[2] != '-') {
        return make_error("Отказ: на третьей позиции должен стоять дефис '-'.");
    }

    int number = 0;
    for (std::size_t i = 3; i < 7; ++i) {
        if (!is_ascii_digit(raw_input[i])) {
            return make_error("Отказ: последние четыре символа должны быть десятичными цифрами (0-9).");
        }
        number = number * 10 + (raw_input[i] - '0');
    }

    // По условию варианта 13 переход через 9999 не определен.
    if (number == 9999) {
        return make_error("Отказ: следующий код после 9999 не определен.");
    }

    // 2. Все проверки внешнего входа пройдены — фиксируем внутренние инварианты перед расчетом.
    const std::string letters(raw_input.substr(0, 2));
    assert(letters.size() == 2);
    assert(number >= 0 && number <= 9998);

    // 3. Выполняем четыре операции варианта 13.
    EvaluationResult result;
    result.ok = true;
    result.letters = letters;
    result.number = number;
    result.formatted_code = letters + "-" + format_number_4(number);
    result.next_code = letters + "-" + format_number_4(number + 1);

    // 4. Постусловия корректного результата (проверяются в Debug, вырезаются в Release).
    assert(result.formatted_code.size() == 7 && result.formatted_code[2] == '-');
    assert(result.next_code.size() == 7 && result.next_code[2] == '-');
    assert(result.ok && result.error.empty());

    return result;
}

}  // namespace srpo::v13