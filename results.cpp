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

static const char* SEP_TEST =
    "+-----+----------------+----------------+----------------+"
    "---------------+---------------+----------------+----+----+"
    "----------------+---------------+\n";

static const char* SEP_MAIN =
    "+-----+----------------+----------------+----------------+"
    "---------------+---------------+----------------+----+----+\n";

void print_test_table(const std::vector<StepResult>& res, std::ostream& out) {
    out << "\n" << SEP_TEST;
    out << "|  i  |       x        |       v        |       v2       |"
           "  v_i - v2_i   |      OLP      |       h        | C1 | C2 |"
           "       u        |  |u_i - v_i|  |\n"
           "|     |                |                |                |"
           "               |               |                |дв|дл|"
           "                |               |\n";
    out << SEP_TEST;

    for (size_t i = 0; i < res.size(); ++i) {
        const auto& r = res[i];
        out << "| " << std::setw(3) << i << " |"
            << fmt_fixed(r.x, 8) << " |"
            << fmt_fixed(r.v, 8) << " |"
            << fmt_fixed(r.v2, 8) << " |"
            << fmt_sci(r.v - r.v2) << " |"
            << fmt_sci(r.err) << " |"
            << fmt_fixed(r.h, 8) << " |"
            << std::setw(2) << r.c1 << " |"
            << std::setw(2) << r.c2 << " |"
            << fmt_fixed(r.exact, 8) << " |"
            << fmt_sci(std::fabs(r.exact - r.v)) << " |\n";
    }
    out << SEP_TEST;
}

void print_main_table(const std::vector<StepResult>& res, std::ostream& out) {
    out << "\n" << SEP_MAIN;
    out << "|  i  |       x        |       v        |       v2       |"
           "  v_i - v2_i   |      OLP      |       h        | C1 | C2 |\n"
           "|     |                |                |                |"
           "               |               |                |дв|дл|\n";
    out << SEP_MAIN;

    for (size_t i = 0; i < res.size(); ++i) {
        const auto& r = res[i];
        out << "| " << std::setw(3) << i << " |"
            << fmt_fixed(r.x, 8) << " |"
            << fmt_fixed(r.v, 8) << " |"
            << fmt_fixed(r.v2, 8) << " |"
            << fmt_sci(r.v - r.v2) << " |"
            << fmt_sci(r.err) << " |"
            << fmt_fixed(r.h, 8) << " |"
            << std::setw(2) << r.c1 << " |"
            << std::setw(2) << r.c2 << " |\n";
    }
    out << SEP_MAIN;
}

void print_statistics(const std::vector<StepResult>& res, bool is_test, std::ostream& out, double b) {
    if (res.empty()) return;

    size_t n = res.size() - 1;
    double xn = res.back().x;
    double max_err = 0.0, x_max_err = 0.0;
    double max_h = 0.0, x_max_h = 0.0;
    double min_h = 1e30, x_min_h = 0.0;
    double max_diff = 0.0, x_max_diff = 0.0;

    int total_c1 = 0;
    int total_c2 = 0;

    for (size_t i = 1; i < res.size(); ++i) {
        const auto& r = res[i];
        double abserr = std::fabs(r.err);
        if (abserr > max_err) { max_err = abserr; x_max_err = r.x; }
        if (r.h > max_h)      { max_h   = r.h;    x_max_h   = r.x; }
        if (r.h < min_h)      { min_h   = r.h;    x_min_h   = r.x; }
        
        if (is_test) {
            double diff = std::fabs(r.exact - r.v);
            if (diff > max_diff) { max_diff = diff; x_max_diff = r.x; }
        }

        total_c1 += r.c1;
        total_c2 += r.c2;
    }

    out << "\nСтатистика:\n";
    out << "  n            = " << n << "\n";
    out << "  b - x_n      = " << std::scientific << std::setprecision(6) << (b - xn) << "\n";
    out << "  max |OLP|    = " << std::scientific << std::setprecision(6) << max_err
        << "  (x = " << std::fixed << std::setprecision(6) << x_max_err << ")\n";
    out << "  max h        = " << std::fixed << std::setprecision(6) << max_h
        << "  (x = " << x_max_h << ")\n";
    out << "  min h        = " << std::fixed << std::setprecision(6) << min_h
        << "  (x = " << x_min_h << ")\n";
    out << "  Удвоений (C1)= " << total_c1 << "\n";
    out << "  Делений (C2) = " << total_c2 << "\n";

    if (is_test) {
        out << "  max |u-v|    = " << std::scientific << std::setprecision(6) << max_diff
            << "  (x = " << std::fixed << std::setprecision(6) << x_max_diff << ")\n";
    }
}