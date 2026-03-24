#include "results.hpp"

static std::string fmt_fixed(double v, int prec = 7) {
    std::ostringstream s;
    s << std::fixed << std::setprecision(prec) << std::setw(14) << v;
    return s.str();
}

static std::string fmt_sci(double v, int prec = 4) {
    std::ostringstream s;
    s << std::scientific << std::setprecision(prec) << std::setw(13) << v;
    return s.str();
}

void print_test_table(const std::vector<StepResult>& res, std::ostream& out) {
    out << "\n";
    out << "+-----+----------------+----------------+----------------+---------------+---------------+----------------+----+----+----------------+---------------+\n";
    out << "|  i  |       x        |       v        |       v2       |  v_i - v2_i   |      OLP      |       h        | C1 | C2 |       u        |  |u_i - v_i|  |\n";
    out << "+-----+----------------+----------------+----------------+---------------+---------------+----------------+----+----+----------------+---------------+\n";
    for (size_t i = 0; i < res.size(); ++i) {
        const auto& r = res[i];
        out << "| " << std::setw(3) << i+1               << " "
            << "| " << fmt_fixed(r.x)                    << " "
            << "| " << fmt_fixed(r.v)                    << " "
            << "| " << fmt_fixed(r.v2)                   << " "
            << "| " << fmt_sci(r.err)                    << " "
            << "| " << fmt_sci(r.err)                    << " "
            << "| " << fmt_fixed(r.h)                    << " "
            << "| " << std::setw(2) << r.c1              << " "
            << "| " << std::setw(2) << r.c2              << " "
            << "| " << fmt_fixed(r.exact)                << " "
            << "| " << fmt_sci(std::fabs(r.exact - r.v)) << " |\n";
    }
    out << "+-----+----------------+----------------+----------------+---------------+---------------+----------------+----+----+----------------+---------------+\n";
}

void print_main_table(const std::vector<StepResult>& res, std::ostream& out) {
    out << "\n";
    out << "+-----+----------------+----------------+----------------+---------------+---------------+----------------+----+----+\n";
    out << "|  i  |       x        |       v        |       v2       |  v_i - v2_i   |      OLP      |       h        | C1 | C2 |\n";
    out << "+-----+----------------+----------------+----------------+---------------+---------------+----------------+----+----+\n";
    for (size_t i = 0; i < res.size(); ++i) {
        const auto& r = res[i];
        out << "| " << std::setw(3) << i+1    << " "
            << "| " << fmt_fixed(r.x)         << " "
            << "| " << fmt_fixed(r.v)         << " "
            << "| " << fmt_fixed(r.v2)        << " "
            << "| " << fmt_sci(r.err)         << " "
            << "| " << fmt_sci(r.err)         << " "
            << "| " << fmt_fixed(r.h)         << " "
            << "| " << std::setw(2) << r.c1   << " "
            << "| " << std::setw(2) << r.c2   << " |\n";
    }
    out << "+-----+----------------+----------------+----------------+---------------+---------------+----------------+----+----+\n";
}

// FIX: принимаем b явно, чтобы вычислить реальную разность b - x_n
void print_statistics(const std::vector<StepResult>& res, bool is_test,
                      std::ostream& out, double b) {
    if (res.empty()) return;

    size_t n = res.size();
    double xn = res.back().x;
    double max_err = 0.0, max_h = 0.0, min_h = 1e100;
    double x_max_err = 0.0, x_max_h = 0.0, x_min_h = 0.0;
    double max_diff = 0.0, x_max_diff = 0.0;

    for (const auto& r : res) {
        double abserr = std::fabs(r.err);
        if (abserr > max_err) { max_err = abserr; x_max_err = r.x; }
        if (r.h > max_h)      { max_h   = r.h;    x_max_h   = r.x; }
        if (r.h < min_h)      { min_h   = r.h;    x_min_h   = r.x; }
        if (is_test) {
            double diff = std::fabs(r.exact - r.v);
            if (diff > max_diff) { max_diff = diff; x_max_diff = r.x; }
        }
    }

    out << "\nСтатистика:\n";
    out << "  n            = " << n << "\n";
    // FIX: правильная разность b - x_n
    out << "  b - x_n      = " << std::scientific << std::setprecision(6)
        << (b - xn) << "\n";
    out << "  max |OLP|    = " << std::scientific << std::setprecision(6) << max_err
        << "  (x = " << std::fixed << std::setprecision(6) << x_max_err << ")\n";
    out << "  Делений шага = " << res.back().c1 << "\n";
    out << "  Удвоений     = " << res.back().c2 << "\n";
    out << "  max h_i      = " << std::fixed    << std::setprecision(10) << max_h
        << "  (x = " << std::setprecision(6) << x_max_h << ")\n";
    out << "  min h_i      = " << std::setprecision(10) << min_h
        << "  (x = " << std::setprecision(6) << x_min_h << ")\n";
    if (is_test) {
        out << "  max |u-v|    = " << std::scientific << std::setprecision(6) << max_diff
            << "  (x = " << std::fixed << std::setprecision(6) << x_max_diff << ")\n";
    }
    out << std::endl;
}
