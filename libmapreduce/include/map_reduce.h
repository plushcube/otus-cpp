#pragma once

#include <iosfwd>
#include <string>
#include <string_view>

namespace map_reduce {

// Читает stdin целиком блоками: построчный ввод через iostream на порядок
// медленнее (около 1.7 с на 6.7 МБ против 0.16 с).
std::string read_stdin();

// Печатает цену (столбец price) для каждой корректной строки CSV-входа.
// Строки, не совпадающие со схемой датасета, пропускаются.
void map_price(std::string_view input, std::ostream &out);

// Печатает среднее арифметическое чисел со входа.
// Строки, которые не являются числами, пропускаются.
void reduce_mean(std::string_view input, std::ostream &out);

// Печатает генеральную дисперсию чисел со входа.
// Строки, которые не являются числами, пропускаются.
void reduce_variance(std::string_view input, std::ostream &out);

} // namespace map_reduce
