#ifndef INTEGRATOR_HPP
#define INTEGRATOR_HPP

#include "utils.hpp"
#include "results.hpp"

// Один шаг метода Рунге-Кутты 4-го порядка
std::vector<double> rk4_step(
    const std::function<std::vector<double>(double, const std::vector<double>&)>& func,
    double x,
    const std::vector<double>& y,
    double h
);

// Интегрирование с постоянным шагом (с вычислением оценки погрешности)
std::vector<StepResult> integrate_fixed(
    const std::function<std::vector<double>(double, const std::vector<double>&)>& func,
    double x0,
    const std::vector<double>& y0,
    double h,
    double xmax,
    int Nmax,
    bool is_test,
    double u0_exact = 0.0
);

// Интегрирование с контролем локальной погрешности (адаптивный шаг)
std::vector<StepResult> integrate_adaptive(
    const std::function<std::vector<double>(double, const std::vector<double>&)>& func,
    double x0,
    const std::vector<double>& y0,
    double h0,
    double xmax,
    double eps,
    int Nmax,
    bool is_test,
    double u0_exact = 0.0
);

#endif // INTEGRATOR_HPP