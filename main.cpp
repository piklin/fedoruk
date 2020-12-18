#include <vector>
#include <cmath>
#include <iostream>

// Настройки
bool is_cube = false;
size_t elements_count = 10;

// задание ГУ
double du_0 = -5;
double u_n = 10;
double start_x = 2;
double end_x = 7;

// Вспомогательное
double element_len = (end_x - start_x) / elements_count;

double A_local_linear[2][2] = {{1 / element_len + 7 * element_len / 6,      -1 / element_len + 7 * element_len / 12},
                               {-1 / element_len + 7 * element_len / 12,    1 / element_len + 7 * element_len / 6}};
double b_local_linear[2] =     {3 * element_len / 4,                        3 * element_len / 4};

double A_local_cube[4][4] =
        {{37 / (10 * element_len) + 4 * element_len / 15, -189 / (40 * element_len) + 33 * element_len / 160, 27 / (20 * element_len) - 3 * element_len / 40, -13 / (40 * element_len) + 19 * element_len / 480},
         {-189 / (40 * element_len) + 33 * element_len / 160, 54 / (5 * element_len) + 27 * element_len / 20, -297 / (40 * element_len) - 27 * element_len / 160, 27 / (20 * element_len) - 3 * element_len / 40},
         {27 / (20 * element_len) - 3 * element_len / 40, -297 / (40 * element_len) + -27 * element_len / 160, 54 / (5 * element_len) + 27 * element_len / 20, -189 / (40 * element_len) + 33 * element_len / 160},
         {-13 / (40 * element_len) + 19 * element_len / 480, 27 / (20 * element_len) - 3 * element_len / 40, -189 / (40 * element_len) + 33 * element_len / 160, 37 / (10 * element_len) + 4 * element_len / 15}};
double b_local_cube[4] = {3 * element_len / 16, 9 * element_len / 16, 9 * element_len / 16, 3 * element_len / 16};

// Точное решение
std::vector<double> solution() {
    std::vector<double> result(elements_count + 1);
    for (size_t i = 0; i < elements_count + 1; i++) {
        double x = start_x + i * element_len;
        result[i] = 3.0/7 + (67*exp(7*sqrt(14)/2) + 5*sqrt(14)*exp(6*sqrt(14)))*exp(-x*sqrt(14)/2)/(7*(1 + exp(5*sqrt(14)))) + (-5*sqrt(14) + 67*exp(5*sqrt(14)/2))*exp(-sqrt(14))*exp(x*sqrt(14)/2)/(7*(1 + exp(5*sqrt(14))));
    }
    return result;
}

template <typename T>
class Matrix {
public:
    Matrix(size_t rows, size_t cols, T value);
    Matrix(const Matrix<T> &matrix);
    std::vector<T> tridiagonal_matrix_algorithm(std::vector<T> &b);
    T& get_elem(size_t r, size_t c);
private:
    std::vector<std::vector<T>> data;
    size_t rows;
    size_t cols;
};

template <typename T>
Matrix<T>::Matrix(size_t rows, size_t cols, T value) : rows(rows), cols(cols) {
    data.resize(rows);
    for (size_t i = 0; i < rows; i++) {
        data[i].resize(cols, value);
    }
}

template <typename T>
Matrix<T>::Matrix(const Matrix<T> &matrix) : rows(matrix.rows), cols(matrix.cols), data(matrix.data) {
}

template <typename T>
T& Matrix<T>::get_elem(size_t r, size_t c) {
    return data[r][c];
}

template <typename T>
std::vector<T> Matrix<T>::tridiagonal_matrix_algorithm(std::vector<T> &b) {

    size_t n = this->rows;
    std::vector<T> alpha(n);
    std::vector<T> beta(n);

    alpha[0] = -this->data[0][1] / this->data[0][0];
    beta[0] = b[0] / this->data[0][0];


    for (size_t i = 1; i < n - 1; i++) {
        double y = this->data[i][i] + this->data[i][i - 1] * alpha[i - 1];
        alpha[i] = -this->data[i][i + 1] / y;
        beta[i] = (b[i] - this->data[i][i - 1] * beta[i - 1]) / y;
    }
    double y = this->data[n - 1][n - 1] + this->data[n - 1][n - 2] * alpha[n - 2];
    beta[n - 1] = (b[n - 1] - this->data[n - 1][n - 2] * beta[n - 2]) / y;

    std::vector<T> x(n);
    x[n - 1] = beta[n - 1];
    for (ssize_t i = n - 2; i >= 0; i--) {
        x[i] = alpha[i] * x[i + 1] + beta[i];
    }

    return x;
}

std::vector<double> linear() {

    // Получение матрицы A ансамблированием и учет граничных условий
    Matrix<double> A(elements_count + 1, elements_count + 1, 0);
    for (size_t i = 0; i < elements_count; i++) {
        for (size_t j = 0; j < 2; j++) {
            for (size_t k = 0; k < 2; k++) {
                A.get_elem(i + j, i + k) += A_local_linear[j][k];
            }
        }
    }
    A.get_elem(elements_count, elements_count) = 1;
    A.get_elem(elements_count - 1, elements_count) = 0;

    // Получение вектора b и учет граничных условий
    std::vector<double> b(elements_count + 1);
    for (size_t i = 1; i < elements_count - 1; i++) {
        b[i] = b_local_linear[0] + b_local_linear[1];
    }
    b[0] = b_local_linear[0] - du_0;
    b[elements_count - 1] = b_local_linear[0] + b_local_linear[1] - A_local_linear[0][1] * u_n;
    b[elements_count] = b_local_linear[1] - A_local_linear[1][1] * u_n;

    // Решение СЛАУ Ax=b
    std::vector<double> x = A.tridiagonal_matrix_algorithm(b);
    x[elements_count] = u_n;
    return x;
}

std::vector<double> cube() {

    // Приведение с момощью метода Гаусса локальной матрицы к виду,
    // необходимому для исключения внутренних элементов
    for (size_t i = 1; i < 3; i++) {
        for (size_t j = 0; j < 4; j++) {
            if (i == j | fabs(A_local_cube[j][i]) < 1e-16) {
                continue;
            }
            double piv = A_local_cube[j][i] / A_local_cube[i][i];
            b_local_cube[j] -= piv * b_local_cube[i];
            for (size_t k = 0; k < 4; k++) {
                A_local_cube[j][k] -= piv * A_local_cube[i][k];
            }
        }
    }

    // Получение матрицы A ансамблированием и учет граничных условий
    Matrix<double> matrix(elements_count + 1, elements_count + 1, 0);
    for (size_t i = 0; i < elements_count; i++) {
        matrix.get_elem(i, i) += A_local_cube[0][0];
        matrix.get_elem(i + 1, i) += A_local_cube[3][0];
        matrix.get_elem(i, i + 1) += A_local_cube[0][3];
        matrix.get_elem(i + 1, i + 1) += A_local_cube[3][3];
    }
    matrix.get_elem(elements_count, elements_count) = 1;
    matrix.get_elem(elements_count - 1, elements_count) = 0;

    // Получение вектора b и учет граничных условий
    std::vector<double> b(elements_count + 1);
    for (size_t i = 1; i < elements_count - 1; i++) {
        b[i] = b_local_cube[0] + b_local_cube[3];
    }
    b[0] = b_local_cube[0] - du_0;
    b[elements_count] = b_local_cube[3] - A_local_cube[3][3] * u_n;
    b[elements_count - 1] = b_local_cube[0] + b_local_cube[3] - A_local_cube[0][3] * u_n;

    // Решение СЛАУ Ax=b
    std::vector<double> x = matrix.tridiagonal_matrix_algorithm(b);
    x[elements_count] = u_n;
    return x;
}

void print_graph(std::vector<double> &res_y, std::string graph_mame) {
    std::vector<double> x(elements_count + 1);
    for (size_t i = 0; i < res_y.size(); i++) {
        x[i] = start_x + i * element_len;
    }
    std::vector<double> y = solution();

    FILE* gnuplot = popen("gnuplot -persist", "w");

    fprintf(gnuplot, "$mfe_res << EOD\n");
    for (size_t i = 0; i < elements_count + 1; i++) {
        fprintf(gnuplot, "%lf %lf\n", x[i], res_y[i]);
    }
    fprintf(gnuplot, "EOD\n");

    fprintf(gnuplot, "$exact << EOD\n");
    for (size_t i = 0; i < elements_count + 1; i++) {
        fprintf(gnuplot, "%lf %lf\n", x[i], y[i]);
    }
    fprintf(gnuplot, "EOD\n");

    fprintf(gnuplot, "set grid\n set title '%s' font \"Helvetica,16\" lt 3 lw 5\n", graph_mame.c_str());
    fprintf(gnuplot, "plot '$mfe_res' using 1:2 with lines title 'MFE (%zu elements)' lc rgb \"blue\" lw 1, '$exact' using 1:2 with lines title 'exact (%zu elements)',\n", elements_count, elements_count);
    fflush(gnuplot);
}

double max_error(std::vector<double> y) {
    std::vector<double> exact_y = solution();
    double max = 0;
    for (size_t i = 0; i < y.size(); i++) {
        double error = fabs(y[i] - exact_y[i]);
        if (error > max) {
            max = error;
        }
    }
    return max;
}

int main() {
    std::cout << "Погнали нахуй!!!!!!!" << std::endl;
    std::vector<double> res;
    if (!is_cube) {
        res = linear();
        print_graph(res, std::string("Linear"));
    } else {
        res = cube();
        print_graph(res, std::string("Cube"));
    }

    std::cout << "Max error between points " << max_error(res) << std::endl;
    return 0;
}
//10:
// max error 0.139618
// max error 1.20512e-05
