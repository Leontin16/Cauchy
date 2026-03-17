#include "results.hpp"

void print_test_table(const std::vector<StepResult>& res, std::ostream& out) {
    out << "\n";
    out << "-----------------------------------------------------------------------------------------------------------------------------\n";
    out << "| i |    x     |    v     |   v2     | v_i - v2_i |   ОЛП    |    h     | C1 | C2 |    u     | |u_i - v_i| |\n";
    out << "-----------------------------------------------------------------------------------------------------------------------------\n";
    for (size_t i = 0; i < res.size(); ++i) {
        const auto& r = res[i];
        out << "|" << std::setw(3) << i+1
            << " | " << std::fixed << std::setprecision(5) << std::setw(8) << r.x
            << " | " << std::setw(8) << r.v
            << " | " << std::setw(8) << r.v2
            << " | " << std::setw(11) << r.err
            << " | " << std::setw(8) << r.err       // ОЛП – та же разность
            << " | " << std::setw(8) << r.h
            << " | " << std::setw(2) << r.c1
            << " | " << std::setw(2) << r.c2
            << " | " << std::setw(8) << r.exact
            << " | " << std::setw(12) << std::fabs(r.exact - r.v) << " |\n";
    }
    out << "-----------------------------------------------------------------------------------------------------------------------------\n";
}

void print_main_table(const std::vector<StepResult>& res, std::ostream& out) {
    out << "\n";
    out << "----------------------------------------------------------------------------------------------------\n";
    out << "| i |    x     |    v     |   v2     | v_i - v2_i |   ОЛП    |    h     | C1 | C2 |\n";
    out << "----------------------------------------------------------------------------------------------------\n";
    for (size_t i = 0; i < res.size(); ++i) {
        const auto& r = res[i];
        out << "|" << std::setw(3) << i+1
            << " | " << std::fixed << std::setprecision(5) << std::setw(8) << r.x
            << " | " << std::setw(8) << r.v
            << " | " << std::setw(8) << r.v2
            << " | " << std::setw(11) << r.err
            << " | " << std::setw(8) << r.err
            << " | " << std::setw(8) << r.h
            << " | " << std::setw(2) << r.c1
            << " | " << std::setw(2) << r.c2 << " |\n";
    }
    out << "----------------------------------------------------------------------------------------------------\n";
}

void print_statistics(const std::vector<StepResult>& res, bool is_test, std::ostream& out) {
    if (res.empty()) return;

    size_t n = res.size();
    double b = res.back().x;
    double max_err = 0.0, max_h = 0.0, min_h = 1e100;
    double x_max_err = 0.0, x_max_h = 0.0, x_min_h = 0.0;
    double max_diff = 0.0; // для теста: |u_i - v_i|
    double x_max_diff = 0.0;

    for (const auto& r : res) {
        double abserr = std::fabs(r.err);
        if (abserr > max_err) {
            max_err = abserr;
        }
        if (r.h > max_h) {
            max_h = r.h;
            x_max_h = r.x;
        }
        if (r.h < min_h) {
            min_h = r.h;
            x_min_h = r.x;
        }
        if (is_test) {
            double diff = std::fabs(r.exact - r.v);
            if (diff > max_diff) {
                max_diff = diff;
                x_max_diff = r.x;
            }
        }
    }

    out << "\nСтатистика:\n";
    out << "n = " << n << "\n";
    out << "b - x_n = " << b << "\n";
    out << "max |ОЛП| = " << std::scientific << max_err << "\n";
    out << "Общее число делений шага = " << (res.empty() ? 0 : res.back().c1) << "\n";
    out << "Общее число удвоений шага = " << (res.empty() ? 0 : res.back().c2) << "\n";
    out << "max h_i = " << std::fixed << std::setprecision(5) << max_h << " при x = " << x_max_h << "\n";
    out << "min h_i = " << min_h << " при x = " << x_min_h << "\n";
    if (is_test) {
        out << "max |u_i - v_i| = " << max_diff << " при x = " << x_max_diff << "\n";
    }
    out << std::endl;
}