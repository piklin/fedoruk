#include <iostream>
#include <vector>

//граничные условия
double u_0 = 0;
double du_n = 1;
double start_x = 3;
double end_x = 14;
//////////////////////////////////////// граничные меняй


template <typename T>
class Matrix {
public:
    Matrix(size_t rows, size_t cols, T value);

    void print();
    T& get_elem(size_t r, size_t c);
    std::vector<T> gauss(std::vector<T> &b);
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
void Matrix<T>::print() {
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            std::cout << data[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

template <typename T>
T& Matrix<T>::get_elem(size_t r, size_t c) {
    return data[r][c];
}

template <typename T>
std::vector<T> Matrix<T>::gauss(std::vector<T> &b) {
    double p = 0;
    for (size_t i = 0; i < this->rows; i++) {
        for (size_t j = i + 1; j < this->cols; j++) {
            if (fabs(this->data[j][i]) < 1e-16) {
                continue;
            }
            p = this->data[j][i] / this->data[i][i];
            b[j] -= p * b[i];
            for (size_t k = 0; k < this->rows; k++) {
                this->data[j][k] -= p * this->data[i][k];
            }
        }
    }


    std::vector<T> res(this->rows);
    for (int i = this->rows - 1; i >= 0; i--) {
        res[i] = b[i];
        for (size_t j = i + 1; j < this->rows; j++) {
            res[i] -= res[j] * this->data[i][j];
        }
        res[i] /= this->data[i][i];
    }

    return res;
}

std::vector<double> solution_for_linear(size_t elements_count) {

    double len = (end_x - start_x) / elements_count;

    Matrix<double> matrix(elements_count + 1, elements_count + 1, 0);
    std::vector<double> b(elements_count + 1);
    //тут матрица
    double l[2][2] = {{5.0 * len + 1.0 / len,       5.0 * len / 2.0 - 1.0 / len},
                      {5.0 * len / 2.0 - 1.0 / len, 5.0 * len + 1.0 /len}};
////////////////////////////////////////матрицу меняй
    for (size_t i = 0; i < elements_count; i++) {
        for (size_t j = 0; j < 2; j++) {
            for (size_t k = 0; k < 2; k++) {
                matrix.get_elem(i + j, i + k) += l[j][k];
            }
        }
    }

    //тут учитываются граничные условия
    matrix.get_elem(0, 0) = 1;
    matrix.get_elem(1, 0) = 0;
    b[0] = 2 * len - l[0][0] * u_0;
    b[1] = 4 * len - l[1][0] * u_0;
    for (size_t i = 2; i < elements_count; i++) {
        b[i] = 4 * len;
    }
    b[elements_count] = 2 * len + du_n;
    ////////////////////////////////////////граничные меняй

    //решаем слау
    std::vector<double> result = matrix.gauss(b);
    result[0] = u_0;
    return result;
}

std::vector<double> solution_for_cube(size_t elements_count) {

    double len = (end_x - start_x) / elements_count;
////////////////////////////////////////матрицы меняй
    double l_b[4] = {len / 2., 3 * len / 2., 3 * len / 2., len/ 2.};
    double l[4][4] =
            {{  37. / (10 * len) + 15 * 8 * len / 105.,
                     -189. / (40 * len) + 15 * 33 * len / 560.,
                     27. / (20 * len) - 15 * 3 * len / 140.,
                     -13. / (40 * len) + 15 * 19 *len / 1680.},
             {   -189. / (40 * len) + 15 * 33 * len / 560.,
                     54. / (5 * len) + 15 * 27 * len / 70.,
                     -297. / (40 * len) + -27 * 15 * len / 560.,
                     27. / (20 * len) - 15 * 3 * len / (140.)},
             {   27. / (20 * len) - 15 * 3 * len / 140.,
                     -297. / (40 * len) - 15 * 27 * len / 560.,
                     54. / (5 * len) + 15 * 27 * len / 70.,
                     -189. / (40 * len) + 15 * 33 * len / 560.},
             {   -13. / (40 * len) + 15 * 19 * len / 1680.,
                     27. / (20 * len) - 15 * 3 * len / 140.,
                     -189. / (40 * len) + 15 * 33 * len / 560.,
                     37. / (10 * len) + 15 * 8 * len / 105.}};


    // Исключение внутренних элементов и получение матриц без них
    double p;
    for (size_t i = 1; i < 3; i++) {
        for (size_t j = 0; j < 4; j++) {
            if (fabs(l[j][i]) < 1e-16) {
                continue;
            }
            if (i == j) {
                continue;
            }
            p = l[j][i] / l[i][i];
            l_b[j] -= p * l_b[i];
            for (size_t k = 0; k < 4; k++) {
                l[j][k] -= p * l[i][k];
            }
        }
    }
    double small_l[2][2] = {{l[0][0], l[0][3]},
                            {l[3][0], l[3][3]}};
    double small_b[2] = {l_b[0], l_b[3]};

    // Создаем и заполняем матрицу и вектор b
    Matrix<double> matrix(elements_count + 1, elements_count + 1, 0);
    for (size_t i = 0; i < elements_count; i++) {
        for (size_t j = 0; j < 2; j++) {
            for (size_t k = 0; k < 2; k++) {
                matrix.get_elem(i + j,i + k) += small_l[j][k];
            }
        }
    }

    std::vector<double> b(elements_count + 1);
    for (size_t i = 2; i < elements_count; i++) {
        b[i] = small_b[1] + small_b[0];
    }
    b[elements_count] = small_b[1] + du_n;

    // Накладываем граничные условия
    matrix.get_elem(0, 0) = 1;
    matrix.get_elem(1, 0) = 0;
    b[0] = small_b[0] - small_l[0][0] * u_0;
    b[1] = small_b[1] + small_b[0] - small_l[1][0] * u_0;
    ////////////////////////////////////////граничные меняй

    // Решаем слау
    std::vector<double> result = matrix.gauss(b);
    result[0] = u_0;
    return result;
}

std::vector<double> exact_solution(size_t elements_count) {
    double len = (end_x - start_x) / (elements_count);

    std::vector<double> res(elements_count + 1);
    for (size_t i = 0; i < elements_count + 1; i++) {
        double x = start_x + i * len;
        res[i] = -29649.312 * exp(-sqrt(15) * x) + 7.30701889e-25 * exp(sqrt(15) * x) + 4.0 / 15.0;
        ////////////////////////////////////////точное меняй меняй
    }
    return res;
}

void print_graph(std::vector<double> &mke_y, std::string graph_mame) {
    std::vector<double> x(mke_y.size());
    double len = (end_x - start_x) / (mke_y.size() - 1);
    for (size_t i = 0; i < mke_y.size(); i++) {
        x[i] = start_x + i * len;
    }
    std::vector<double> exact_y = exact_solution(mke_y.size() - 1);

    FILE* gp = popen("gnuplot -persist", "w");

    fprintf(gp, "$mke_res << EOD\n");
    for (size_t i = 0; i < mke_y.size(); i++) {
        fprintf(gp, "%lf %lf\n", x[i], mke_y[i]);
    }
    fprintf(gp, "EOD\n");

    fprintf(gp, "$exact << EOD\n");
    for (size_t i = 0; i < mke_y.size(); i++) {
        fprintf(gp, "%lf %lf\n", x[i], exact_y[i]);
    }
    fprintf(gp, "EOD\n");

    fprintf(gp, "set title '%s' font \"Helvetica,20\" lt 3 lw 5\n", graph_mame.c_str());
    fprintf(gp, "plot '$mke_res' using 1:2 with lines title 'exact (%zu elements)' lc rgb \"red\" lw 1, '$exact' using 1:2 with lines title 'MKE (%zu elements)',\n", mke_y.size() - 1, mke_y.size() - 1);
    fflush(gp);
    fclose(gp);
}

double max_error(std::vector<double> y) {
    std::vector<double> exact_y = exact_solution(y.size() - 1);

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
    std::cout << "Enter elements count and type (lin = 0, cube = 1)" << std::endl;
    size_t elements_count = 0, type = 0;
    std::cin >> elements_count >> type;

    std::vector<double> res;
    if (!type) {
        res = solution_for_linear(elements_count);
        print_graph(res, "Linear and exact");
    } else {
        res = solution_for_cube(elements_count);
        print_graph(res, "Cube and exact");
    }

    std::cout << "max error " << max_error(res) << std::endl;

    return 0;
}

