#include "models.hpp"

std::vector<double> test_rhs(double x, const std::vector<double>& y) {
    // y[0] = u
    return { -1.5 * y[0] };
}

std::vector<double> main_rhs(double x, const std::vector<double>& y, const MainParams& p) {
    double u = y[0];
    double v = y[1];
    double du = v;
    double dv = -(p.c / p.m) * v - (p.k / p.m) * u - (p.ks / p.m) * u * u * u;
    return { du, dv };
}