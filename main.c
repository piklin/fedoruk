#include <stdio.h>
#include <stdlib.h>

#define h 0.01
#define M 2
#define N 8
#define dt 0.5
#define TIME 15
#define K 1
#define Tb 100
#define Tr 20

double p = dt * K / (h * h);

void solver(double *prev) {
    size_t cm = (size_t)(M / h + 1);
    size_t cn = (size_t)(N / h + 1);

    double *alpha = malloc(cm * sizeof(double));
    double *beta = malloc(cm * sizeof(double));
    for (size_t i = 1; i < cn - 1; i++) {

        alpha[1] = 1;
        beta[1] = 0;
        for(size_t k = 2; k < cm; k++) {
            alpha[k] = p / (1 + 2 * p - p * alpha[k - 1]);
            beta[k] = (prev[i * cm + k - 1] + p * beta[k - 1]) / (1 + 2 * p - p * alpha[k - 1]);
        }

        prev[(i + 1) * cm - 1] = Tr;
        for(int k = cm - 2; k >= 0; k--) {
            prev[i * cm + k] = alpha[k + 1] * prev[i * cm + k + 1] + beta[k + 1];
        }
    }

    free(alpha);
    free(beta);
    alpha = malloc(cn * sizeof(double));
    beta = malloc(cn * sizeof(double));
    for (size_t i = 1; i < cm - 1; i++) {

        alpha[1] = 1;
        beta[1] = 0;
        for(size_t k = 2; k < cn; k++) {
            alpha[k] = p / (1 + 2 * p - p * alpha[k - 1]);
            beta[k] = (prev[i + cm * (k - 1)] + p * beta[k - 1]) / (1 + 2 * p - p * alpha[k - 1]);
        }

        prev[i + cm * (cn - 1)] = Tb;
        for(int k = cn - 2; k >= 0; k--) {
            prev[i + cm * k] = alpha[k + 1] * prev[i + cm * (k + 1)] + beta[k + 1];
            if (i == 1) {
                prev[(i - 1) + cm * k] = prev[i + cm * k];
            }
        }
    }


    free(alpha);
    free(beta);
}

int main() {
    size_t cm = (size_t)(M / h + 1);
    size_t cn = (size_t)(N / h + 1);

    FILE *gnuplot = popen("gnuplot -persist", "w");
    if (gnuplot == NULL) {
        printf("gnuplot error\n");
        exit(EXIT_FAILURE);
    }
    fprintf(gnuplot,  "set cbrange [0:100]\nset yrange [0:%zu]\nset xrange [0:%zu]\nset size ratio %d\n", N, M, N/M);
    fprintf(gnuplot, "set palette defined ( 0 0 0 1, 0.2 0 1 1, 0.4 0 1 0, 0.6 1 1 0, 0.8 1 0.6471 0, 1 1 0 0 )\n"
                     "set pm3d scansforward ftriangles map \n");
    fprintf(gnuplot, "splot '-'\n");


    double *matrix = malloc(cm * cn * sizeof(double));
    for (size_t i = 0; i < cm; i++) {
        matrix[(cn - 1) * cm + i] = 100;
    }
    for (size_t i = 0; i < cn; i++) {
        matrix[(cm - 1) + i * cm] = 20;
    }

    for (size_t i = 0; i < TIME / dt; i++) {
        solver(matrix);
    }


    for(int i = 0; i <= cn; i++) {
        int p = cn - i - 1;
        for (size_t k = 0; k <= cm; k++) {
            fprintf(gnuplot, "%lf %lf %lf\n", k * h, p * h, matrix[i * cm + k]);
            //fprintf(stdout, "%zu %zu %lf\n", k, p, matrix[i * cm + k]);
        }
        fprintf(gnuplot, "\n");
    }
    fprintf(gnuplot, "e\n");
    fflush(gnuplot);
    //fprintf(gnuplot,"pause(0.2)\n");

    return 0;
}

