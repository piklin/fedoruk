#include <stdio.h>
#include <stdlib.h>

#define h 0.1
#define M 8
#define N 2
#define dt 1
#define TIME 15

void solver() {
    for (size_t i = 0; i < N; i++) {
        double *alpha = malloc((M - 1) * sizeof(double));
        double *beta = malloc((M - 1) * sizeof(double));

        alpha[0] = prev;
        beta[0] =

        for (size_t k = 1; k < M; k++) {

        }
    }
}

int main() {
    size_t cm = (size_t)(M / h + 1);
    size_t cn = (size_t)(N / h + 1);


    double *prev = malloc(cm * cn * sizeof(double));
    double *curr = malloc(cm * cn * sizeof(double));
    for (size_t i = 0; i < cm; i++) {
        prev[(cn - 1) * cm + i] = 100;
    }
    for (size_t i = 0; i < cn; i++) {
        prev[(cm - 1) + i * cm] = 20;
    }

    for (size_t i = 0; i < TIME / dt; i++) {
        //solver();
    }

}

