#include "integrator.hpp"

static bool has_bad_values(const std::vector<double>& v) {
    for (double x : v)
        if (!std::isfinite(x)) return true;
    return false;
}

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

    {
        StepResult res;
        res.x = x;
        res.y = y;
        res.v = y[0];
        res.v2 = y[0];
        res.err = 0.0;
        res.h = h;
        res.c1 = 0;
        res.c2 = 0;
        if (is_test) res.exact = u0_exact;
        results.push_back(res);
    }

    int steps = 0;
    while (x < xmax && steps < Nmax) {
        double current_h = h;
        if (x + current_h > xmax) current_h = xmax - x;

        std::vector<double> y_full = rk4_step(func, x, y, current_h);
        std::vector<double> y_half1 = rk4_step(func, x, y, 0.5 * current_h);
        std::vector<double> y_half2 = rk4_step(func, x + 0.5 * current_h, y_half1, 0.5 * current_h);

        double err = std::fabs(y_full[0] - y_half2[0]) / 15.0;

        x += current_h;
        y = y_half2;
        steps++;

        StepResult res;
        res.x = x;
        res.y = y;
        res.v = y_half2[0];
        res.v2 = y_full[0];
        res.err = err;
        res.h = current_h;
        res.c1 = 0;
        res.c2 = 0;
        if (is_test) res.exact = u0_exact * std::exp(-1.5 * x);
        results.push_back(res);

        if (has_bad_values(y)) break;
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

    {
        StepResult res;
        res.x = x;
        res.y = y;
        res.v = y[0];
        res.v2 = y[0];
        res.err = 0.0;
        res.h = h;
        res.c1 = 0;
        res.c2 = 0;
        if (is_test) res.exact = u0_exact;
        results.push_back(res);
    }

    int total_steps = 0;

    while (x < xmax && total_steps < Nmax) {
        double remain = xmax - x;
        if (h > remain) h = remain;

        double current_h = h;
        bool accepted = false;
        double err = 0.0;
        std::vector<double> y1, y2;
        
        bool halved_this_step = false;
        int step_c2 = 0; // Счётчик делений для конкретного узла

        while (!accepted && total_steps < Nmax) {
            y1 = rk4_step(func, x, y, current_h);
            std::vector<double> yh = rk4_step(func, x, y, 0.5 * current_h);
            y2 = rk4_step(func, x + 0.5 * current_h, yh, 0.5 * current_h);

            if (has_bad_values(y2)) {
                current_h /= 2.0;
                step_c2++;
                halved_this_step = true;
                continue;
            }

            err = std::fabs(y1[0] - y2[0]) / 15.0; 

            if (err <= eps) {
                accepted = true;
            } else {
                current_h /= 2.0;
                step_c2++;
                halved_this_step = true;
                if (current_h < 1e-15) {
                    accepted = true; // Защита от бесконечного цикла
                }
            }
        }

        int step_c1 = 0; // Счётчик удвоений для конкретного узла
        {
            double doubled = current_h * 2.0;
            if (!halved_this_step && err < eps / 32.0 && (x + doubled <= xmax + 1e-12)) {
                h = doubled;
                step_c1 = 1; 
            } else {
                h = current_h;
            }
        }

        x += current_h;
        y = y2;
        total_steps++;

        StepResult res;
        res.x     = x;
        res.y     = y;
        res.v     = y2[0];
        res.v2    = y1[0];
        res.err   = err;
        res.h     = current_h;
        res.c1    = step_c1;
        res.c2    = step_c2;
        if (is_test)
            res.exact = u0_exact * std::exp(-1.5 * x);
        
        results.push_back(res);

        if (has_bad_values(y)) break;
    }

    return results;
}