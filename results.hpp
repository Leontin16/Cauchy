#ifndef RESULTS_HPP
#define RESULTS_HPP

#include "utils.hpp"

// Структура для хранения результатов одного шага интегрирования
struct StepResult {
    double x;               // координата (время)
    std::vector<double> y;  // принятое состояние (уточнённое)
    double v;               // первая компонента с шагом h
    double v2;              // первая компонента с половинным шагом
    double err;             // оценка локальной погрешности (v - v2)
    double h;               // использованный шаг
    int c1;                 // накопленное число делений шага
    int c2;                 // накопленное число удвоений шага
    double exact;           // точное решение (только для тестовой задачи)
};

// Печать таблицы для тестовой задачи
void print_test_table(const std::vector<StepResult>& res, std::ostream& out);

// Печать таблицы для основной задачи
void print_main_table(const std::vector<StepResult>& res, std::ostream& out);

// Печать итоговой статистики
void print_statistics(const std::vector<StepResult>& res, bool is_test, std::ostream& out);

#endif // RESULTS_HPP