#ifndef MODELS_HPP
#define MODELS_HPP

#include "utils.hpp"

// Параметры основной задачи (можно менять)
struct MainParams {
    double m;   // масса (Н·с²/см)
    double c;   // демпфирование (Н·с/см²)
    double k;   // линейная жёсткость (Н/см)
    double ks;  // нелинейная жёсткость (Н/см³)

    MainParams() : m(0.01), c(0.15), k(2.0), ks(2.0) {}
};

// Правая часть тестовой задачи du/dx = -1.5 * u
std::vector<double> test_rhs(double x, const std::vector<double>& y);

// Правая часть основной задачи (осциллятор с нелинейной пружиной)
std::vector<double> main_rhs(double x, const std::vector<double>& y, const MainParams& params);

// Обёртка для передачи в интегратор (удобно использовать лямбды в main, но можно и так)
// Оставим право выбора за main: там просто создадим нужную функцию через лямбду.

#endif // MODELS_HPP