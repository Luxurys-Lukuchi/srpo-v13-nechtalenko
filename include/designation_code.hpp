#ifndef SRPO_V13_DESIGNATION_CODE_HPP
#define SRPO_V13_DESIGNATION_CODE_HPP

#include <string>
#include <string_view>

namespace srpo::v13 {

// Результат обработки одной записи кода обозначения формата LL-DDDD.
// Все значения и текст отказа формируются строго внутри предметной библиотеки.
struct EvaluationResult {
    bool ok = false;             // true — вход корректен, false — зафиксирован отказ
    std::string letters;         // Операция 1: буквенная часть (две заглавные латинские буквы, например "AB")
    int number = 0;              // Операция 2: числовая часть как целое число (0..9999)
    std::string formatted_code;  // Операция 3: строка кода с числом ровно из четырех цифр ("AB-0001")
    std::string next_code;       // Операция 4: следующий код (число увеличено на 1, "AB-0002")
    std::string error;           // Текст отказа (заполняется только при ok == false)
};

// Проверяет входную строку формата LL-DDDD и выполняет все четыре операции варианта 13.
// Неверный вход и переход через 9999 отвергаются явно через поле error без исключений наружу.
EvaluationResult evaluate_code(std::string_view raw_input);

}  // namespace srpo::v13

#endif  // SRPO_V13_DESIGNATION_CODE_HPP