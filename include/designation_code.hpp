#pragma once

#include <string>
#include <string_view>
#include <utility>

namespace srpo::v13 {

/// Обобщенный результат операции библиотеки: либо значение, либо текст отказа.
/// Исключения наружу не выбрасываются — ошибки передаются явно через ok == false.
template <typename T>
struct Result {
    bool ok = false;    ///< true при успешном выполнении, false при отказе
    T value{};          ///< Полезный результат (актуален только при ok == true)
    std::string error;  ///< Текст отказа, сформированный библиотекой (при ok == false)

    /// Создает успешный результат с переданным значением.
    static Result success(T val) {
        return Result{true, std::move(val), {}};
    }

    /// Создает результат с отказом и поясняющим сообщением.
    static Result failure(std::string message) {
        return Result{false, T{}, std::move(message)};
    }
};

/// Разобранная и проверенная запись кода обозначения (формат LL-DDDD).
struct DesignationCode {
    std::string letters; ///< Две заглавные латинские буквы ('A'..'Z')
    int number = 0;      ///< Числовая часть в диапазоне 0..9999
};

/// Итоговый отчет по всем четырем операциям карточки варианта 13.
struct EvaluationReport {
    std::string letters;        ///< Операция 1: буквенная часть (например, "AB")
    int number = 0;             ///< Операция 2: числовая часть (например, 42)
    std::string formatted_code; ///< Операция 3: строка кода с 4 цифрами (например, "AB-0042")
    std::string next_code;      ///< Операция 4: следующий код (например, "AB-0043")
};

/// Разбирает входную строку (одно поле без пробелов) в структуру DesignationCode.
/// Возвращает отказ, если строка пуста, содержит пробелы или не соответствует шаблону LL-DDDD.
Result<DesignationCode> parse_code(std::string_view raw);

/// Операция 1: возвращает буквенную часть кода (две заглавные латинские буквы).
Result<std::string> get_letters(const DesignationCode& code);

/// Операция 2: возвращает числовую часть кода как целое число из диапазона 0..9999.
Result<int> get_number(const DesignationCode& code);

/// Операция 3: формирует каноническую строку кода с ведущими нулями (ровно 4 цифры).
Result<std::string> format_code(const DesignationCode& code);

/// Операция 4: вычисляет следующий код (числовая часть + 1).
/// Возвращает отказ при попытке перейти через 9999.
Result<std::string> next_code(const DesignationCode& code);

/// Выполняет полный цикл для оболочек (CLI и будущей GUI):
/// разбирает входную строку и вычисляет все четыре операции карточки.
Result<EvaluationReport> evaluate_code(std::string_view raw);

} // namespace srpo::v13