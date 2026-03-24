#ifndef RESULTS_HPP
#define RESULTS_HPP

#include "utils.hpp"
#include <sstream>

struct StepResult {
    double x;
    std::vector<double> y;
    double v;
    double v2;
    double err;
    double h;
    int c1;
    int c2;
    double exact;
};

void print_test_table(const std::vector<StepResult>& res, std::ostream& out);
void print_main_table(const std::vector<StepResult>& res, std::ostream& out);
// FIX: добавлен параметр b для правильного вычисления b - x_n
void print_statistics(const std::vector<StepResult>& res, bool is_test,
                      std::ostream& out, double b);

#endif
