#include "utils.hpp"
#include "results.hpp"
#include "models.hpp"
#include "integrator.hpp"
#include <clocale>
#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);   // UTF-8 для вывода
    SetConsoleCP(65001);          // UTF-8 для ввода (если нужен русский ввод)
#endif
    setlocale(LC_ALL, "ru_RU.UTF-8");
    cout << "Лабораторная работа №1. Численное решение задачи Коши\n";
    
    int problem_type;      // 1 - тестовая, 2 - основная
    int method_type;       // 1 - постоянный шаг, 2 - адаптивный
    double a, b, h0, eps = 0.0;
    int Nmax;

    cout << "Выберите задачу:\n";
    cout << "  1 - тестовая (du/dx = -1.5 u)\n";
    cout << "  2 - основная (осциллятор с нелинейной пружиной)\n";
    cout << "Ваш выбор: ";
    cin >> problem_type;

    cout << "Выберите метод:\n";
    cout << "  1 - постоянный шаг\n";
    cout << "  2 - адаптивный шаг (контроль погрешности)\n";
    cout << "Ваш выбор: ";
    cin >> method_type;

    cout << "Введите левую границу a: ";
    cin >> a;
    cout << "Введите правую границу b: ";
    cin >> b;
    cout << "Введите начальный шаг h0: ";
    cin >> h0;
    if (method_type == 2) {
        cout << "Введите допустимую локальную погрешность eps: ";
        cin >> eps;
    }
    cout << "Введите максимальное число итераций Nmax: ";
    cin >> Nmax;

    vector<double> y0;
    function<vector<double>(double, const vector<double>&)> func;
    bool is_test = (problem_type == 1);
    double u0_exact = 0.0;

    if (is_test) {
        cout << "Введите начальное значение u0 для тестовой задачи: ";
        cin >> u0_exact;
        y0 = { u0_exact };
        func = test_rhs;   // используем функцию из models
    } else {
        MainParams params;  // можно при желании ввести с клавиатуры
        y0 = { 10.0, 0.0 }; // u(0)=10 см, u'(0)=0
        // Лямбда, захватывающая params
        func = [params](double x, const vector<double>& y) -> vector<double> {
            return main_rhs(x, y, params);
        };
    }

    // Интегрирование
    vector<StepResult> results;
    if (method_type == 1) {
        results = integrate_fixed(func, a, y0, h0, b, Nmax, is_test, u0_exact);
    } else {
        results = integrate_adaptive(func, a, y0, h0, b, eps, Nmax, is_test, u0_exact);
    }

    // Вывод в консоль
    if (is_test) {
        print_test_table(results, cout);
    } else {
        print_main_table(results, cout);
    }
    print_statistics(results, is_test, cout, b);

    // Сохранение в файл для графиков
    string filename;
    if (is_test) {
        filename = (method_type == 1) ? "test_const.txt" : "test_adapt.txt";
    } else {
        filename = (method_type == 1) ? "main_const.txt" : "main_adapt.txt";
    }

    ofstream fout(filename);
    if (!fout) {
        cerr << "Не удалось открыть файл для записи: " << filename << endl;
    } else {
        if (is_test) {
            fout << "# x\tv\tv2\terr\th\texact\n";
            for (const auto& r : results) {
                fout << r.x << "\t" << r.v << "\t" << r.v2 << "\t"
                     << r.err << "\t" << r.h << "\t" << r.exact << "\n";
            }
        } else {
            fout << "# x\tu\tv\tu_with_step_h\tu_with_half_step\terr\th\n";
            for (const auto& r : results) {
                fout << r.x << "\t"
                     << r.y[0] << "\t" << r.y[1] << "\t"
                     << r.v << "\t" << r.v2 << "\t"
                     << r.err << "\t" << r.h << "\n";
            }
        }
        cout << "Данные для графиков сохранены в файл " << filename << endl;
    }

    return 0;
}