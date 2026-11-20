#include "designation_code.hpp"

#include <cstddef>

namespace srpo::v13 {

namespace {

// Проверяем диапазоны ASCII напрямую, чтобы не зависеть от системной локали
// (в отличие от std::isupper / std::isdigit, поведение которых зависит от locale и char).
bool is_upper_latin(char ch) {
    return ch >= 'A' && ch <= 'Z';
}

bool is_digit(char ch) {
    return ch >= '0' && ch <= '9';
}

bool is_whitespace(char ch) {
    return ch == ' ' || ch == '\t' || ch == '\n' ||
           ch == '\r' || ch == '\v' || ch == '\f';
}

// Защитная проверка инвариантов структуры на случай,
// если в модульных тестах или GUI передадут DesignationCode в обход parse_code.
bool is_valid_record(const DesignationCode& code) {
    if (code.letters.size() != 2) {
        return false;
    }
    if (!is_upper_latin(code.letters[0]) || !is_upper_latin(code.letters[1])) {
        return false;
    }
    return code.number >= 0 && code.number <= 9999;
}

// Форматирует целое число 0..9999 ровно в 4 десятичные цифры с ведущими нулями
// без подключения тяжелых потоков ввода-вывода (<sstream>/<iomanip>).
std::string format_number_4(int number) {
    std::string digits(4, '0');
    digits[0] = static_cast<char>('0' + (number / 1000) % 10);
    digits[1] = static_cast<char>('0' + (number / 100) % 10);
    digits[2] = static_cast<char>('0' + (number / 10) % 10);
    digits[3] = static_cast<char>('0' + number % 10);
    return digits;
}

} // namespace

Result<DesignationCode> parse_code(std::string_view raw) {
    if (raw.empty()) {
        return Result<DesignationCode>::failure(
            "Отказ: пустой ввод (ожидается формат LL-DDDD, например AB-1234).");
    }

    // По условию варианта вход — одно поле строго без пробельных символов.
    for (char ch : raw) {
        if (is_whitespace(ch)) {
            return Result<DesignationCode>::failure(
                "Отказ: вход должен состоять из одного поля без пробелов.");
        }
    }

    // Фиксированный формат: 2 буквы + 1 дефис + 4 цифры = ровно 7 символов.
    if (raw.size() != 7) {
        return Result<DesignationCode>::failure(
            "Отказ: неверная длина кода (ожидается ровно 7 символов в формате LL-DDDD).");
    }

    if (!is_upper_latin(raw[0]) || !is_upper_latin(raw[1])) {
        return Result<DesignationCode>::failure(
            "Отказ: первые два символа должны быть заглавными латинскими буквами (A-Z).");
    }

    if (raw[2] != '-') {
        return Result<DesignationCode>::failure(
            "Отказ: третий символ должен быть дефисом '-'.");
    }

    for (std::size_t i = 3; i < 7; ++i) {
        if (!is_digit(raw[i])) {
            return Result<DesignationCode>::failure(
                "Отказ: после дефиса должны стоять ровно четыре десятичные цифры (0-9).");
        }
    }

    // Переводим 4 проверенных символа-цифры в целое число 0..9999.
    const int number = (raw[3] - '0') * 1000 +
                       (raw[4] - '0') * 100 +
                       (raw[5] - '0') * 10 +
                       (raw[6] - '0');

    DesignationCode parsed{std::string(raw.substr(0, 2)), number};
    return Result<DesignationCode>::success(std::move(parsed));
}

Result<std::string> get_letters(const DesignationCode& code) {
    if (!is_valid_record(code)) {
        return Result<std::string>::failure(
            "Отказ: некорректная запись кода обозначения.");
    }
    return Result<std::string>::success(code.letters);
}

Result<int> get_number(const DesignationCode& code) {
    if (!is_valid_record(code)) {
        return Result<int>::failure(
            "Отказ: некорректная запись кода обозначения.");
    }
    return Result<int>::success(code.number);
}

Result<std::string> format_code(const DesignationCode& code) {
    if (!is_valid_record(code)) {
        return Result<std::string>::failure(
            "Отказ: некорректная запись кода обозначения.");
    }
    return Result<std::string>::success(code.letters + "-" + format_number_4(code.number));
}

Result<std::string> next_code(const DesignationCode& code) {
    if (!is_valid_record(code)) {
        return Result<std::string>::failure(
            "Отказ: некорректная запись кода обозначения.");
    }
    // По контракту переход через 9999 не определен и является явным отказом.
    if (code.number >= 9999) {
        return Result<std::string>::failure(
            "Отказ: следующий код после 9999 не определен.");
    }
    return Result<std::string>::success(code.letters + "-" + format_number_4(code.number + 1));
}

Result<EvaluationReport> evaluate_code(std::string_view raw) {
    const auto parsed = parse_code(raw);
    if (!parsed.ok) {
        return Result<EvaluationReport>::failure(parsed.error);
    }

    // Последовательно выполняем все 4 операции карточки; при отказе любой из них
    // (например, next_code для 9999) возвращаем ошибку без частичных результатов.
    const auto letters = get_letters(parsed.value);
    if (!letters.ok) {
        return Result<EvaluationReport>::failure(letters.error);
    }

    const auto number = get_number(parsed.value);
    if (!number.ok) {
        return Result<EvaluationReport>::failure(number.error);
    }

    const auto formatted = format_code(parsed.value);
    if (!formatted.ok) {
        return Result<EvaluationReport>::failure(formatted.error);
    }

    const auto next = next_code(parsed.value);
    if (!next.ok) {
        return Result<EvaluationReport>::failure(next.error);
    }

    EvaluationReport report{
        letters.value,
        number.value,
        formatted.value,
        next.value
    };
    return Result<EvaluationReport>::success(std::move(report));
}

} // namespace srpo::v13