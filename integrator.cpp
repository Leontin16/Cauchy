#include "integrator.hpp"

std::vector<double> rk4_step(
    const std::function<std::vector<double>(double, const std::vector<double>&)>& func,
    double x,
    const std::vector<double>& y,
    double h)
{
    int n = y.size();
    std::vector<double> k1(n), k2(n), k3(n), k4(n), ytemp(n);

    k1 = func(x, y);
    for (int i = 0; i < n; ++i) ytemp[i] = y[i] + 0.5 * h * k1[i];
    k2 = func(x + 0.5 * h, ytemp);
    for (int i = 0; i < n; ++i) ytemp[i] = y[i] + 0.5 * h * k2[i];
    k3 = func(x + 0.5 * h, ytemp);
    for (int i = 0; i < n; ++i) ytemp[i] = y[i] + h * k3[i];
    k4 = func(x + h, ytemp);

    std::vector<double> ynext(n);
    for (int i = 0; i < n; ++i)
        ynext[i] = y[i] + (h / 6.0) * (k1[i] + 2.0 * k2[i] + 2.0 * k3[i] + k4[i]);
    return ynext;
}

std::vector<StepResult> integrate_fixed(
    const std::function<std::vector<double>(double, const std::vector<double>&)>& func,
    double x0,
    const std::vector<double>& y0,
    double h,
    double xmax,
    int Nmax,
    bool is_test,
    double u0_exact)
{
    std::vector<StepResult> results;
    double x = x0;
    std::vector<double> y = y0;
    int step_count = 0;

    while (step_count < Nmax) {
        // FIX: остаток до правой границы; если он пренебрежимо мал — стоп
        double remain = xmax - x;
        if (remain <= h * 1e-10) break;

        double h_actual = std::min(h, remain);

        auto y1   = rk4_step(func, x, y, h_actual);
        auto ymid = rk4_step(func, x, y, h_actual / 2.0);
        auto y2   = rk4_step(func, x + h_actual / 2.0, ymid, h_actual / 2.0);

        double v  = y1[0];
        double v2 = y2[0];
        double err = v - v2;

        double exact = 0.0;
        if (is_test)
            exact = u0_exact * std::exp(-1.5 * (x + h_actual));

        StepResult res;
        res.x   = x + h_actual;
        res.y   = y2;
        res.v   = v;
        res.v2  = v2;
        res.err = err;
        res.h   = h_actual;
        res.c1  = 0;
        res.c2  = 0;
        res.exact = exact;

        results.push_back(res);

        x = res.x;
        y = y2;
        ++step_count;
    }

    return results;
}

std::vector<StepResult> integrate_adaptive(
    const std::function<std::vector<double>(double, const std::vector<double>&)>& func,
    double x0,
    const std::vector<double>& y0,
    double h0,
    double xmax,
    double eps,
    int Nmax,
    bool is_test,
    double u0_exact)
{
    std::vector<StepResult> results;
    double x = x0;
    std::vector<double> y = y0;
    double h = h0;
    int step_count = 0;
    int total_deletions = 0;
    int total_doublings = 0;

    while (step_count < Nmax) {
        double remain = xmax - x;
        if (remain <= h * 1e-10) break;

        double current_h = std::min(h, remain);
        bool accepted = false;
        std::vector<double> y1, y2;
        double err = 0.0;

        while (!accepted) {
            y1 = rk4_step(func, x, y, current_h);
            auto ymid = rk4_step(func, x, y, current_h / 2.0);
            y2 = rk4_step(func, x + current_h / 2.0, ymid, current_h / 2.0);

            err = std::fabs(y1[0] - y2[0]);

            if (err <= eps) {
                accepted = true;
                // FIX: удвоение засчитываем только если удвоенный шаг
                // реально войдёт в интервал (не будет сразу обрезан)
                double doubled = current_h * 2.0;
                if (err < eps / 10.0 && doubled < (xmax - x) * (1.0 - 1e-10)) {
                    h = doubled;
                    ++total_doublings;
                } else {
                    h = current_h;
                }
            } else {
                current_h /= 2.0;
                ++total_deletions;
                if (current_h < 1e-12) {
                    std::cerr << "Внимание: шаг стал слишком маленьким, принудительное принятие.\n";
                    accepted = true;
                    h = current_h;
                }
            }
        }

        double exact = 0.0;
        if (is_test)
            exact = u0_exact * std::exp(-1.5 * (x + current_h));

        StepResult res;
        res.x   = x + current_h;
        res.y   = y2;
        res.v   = y1[0];
        res.v2  = y2[0];
        res.err = err;
        res.h   = current_h;
        res.c1  = total_deletions;
        res.c2  = total_doublings;
        res.exact = exact;

        results.push_back(res);

        x = res.x;
        y = y2;
        ++step_count;
    }

    return results;
}
