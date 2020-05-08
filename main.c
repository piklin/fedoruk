#include "main.h"

int is_ok_knot(ssize_t m, ssize_t n){
    if (m >= 0 && m < M && n >= 0 && n < N) {
        return 1;
    }
    return 0;
}

int volts_solver(size_t m, size_t n) {
    for (size_t i = 0; i < grid.v_count; i++) {
        if (grid.volts[i].x == m && grid.volts[i].y == n) {
            grid.cur[m][n] = grid.volts->U;
            return 1;
        }
    }
    return 0;
}

int currs_solver(ssize_t m, ssize_t n) {
    for (size_t i = 0; i < grid.c_count; i++) {
        if (grid.currs[i].x == m && grid.currs[i].y == n) {
            size_t k = 0;
            double sum = 0;
            if (is_ok_knot((ssize_t)m - 1, n)) {
                k++;
                sum += grid.prev[(ssize_t)m - 1][n];
            }
            if (is_ok_knot(m + 1, n)) {
                k++;
                sum += grid.prev[m + 1][n];
            }
            if (is_ok_knot(m, (ssize_t)n - 1)) {
                k++;
                sum += grid.prev[m][(ssize_t)n - 1];
            }
            if (is_ok_knot(m, n + 1)) {
                k++;
                sum += grid.prev[m][n + 1];
            }
            grid.cur[m][n] = (grid.currs[i].I * R + sum) / k;
            return 1;
        }
    }
    return 0;
}

void point_solver(size_t i) {
    size_t m = i / N;
    size_t n = i % N;
    if (!currs_solver(m, n) && !volts_solver(m, n)) {
        size_t k = 0;
        double sum = 0;
        if (is_ok_knot((ssize_t)m - 1, n)) {
            k++;
            sum += grid.prev[(ssize_t)m - 1][n];
        }
        if (is_ok_knot(m + 1, n)) {
            k++;
            sum += grid.prev[m + 1][n];
        }
        if (is_ok_knot(m, (ssize_t)n - 1)) {
            k++;
            sum += grid.prev[m][(ssize_t)n - 1];
        }
        if (is_ok_knot(m, (n + 1))) {
            k++;
            sum += grid.prev[m][n + 1];
        }
        grid.cur[m][n] = h * (sum - k * grid.prev[m][n]) / (C * R) + grid.prev[m][n];
    }
}

void *solver(void *arg) {
    Thread *thread = (Thread *) arg;
    while (!is_done) {
        for (size_t i = thread->start; i <= thread->end; i++) {
            point_solver(i);
        }
        pthread_barrier_wait(&barr);
    }
    return NULL;
}

int grid_inicialize(Grid *g) {
    g->prev = calloc(M, sizeof(double *));
    g->cur = calloc(M, sizeof(double *));
    g->prev[0] = (double *)calloc(M * N, sizeof(double));
    g->cur[0] = (double *)calloc(M * N, sizeof(double));
    for (size_t i = 1; i < M; i++) {
        g->cur[i] = g->cur[i - 1] + N;
        g->prev[i] = g->prev[i - 1] + N;
    }

    if (!g->prev || !g->cur) {
        printf("Alloc error\n");
        return -1;
    }

    g->v_count = 2;
    g->c_count = 2;
    g->currs = calloc(g->c_count, sizeof(CurrS));
    g->volts = calloc(g->v_count, sizeof(VoltS));
    if (!g->currs || !g->volts) {
        printf("Alloc error\n");
        return -1;
    }

    g->currs[0].x = 3;
    g->currs[0].y = 2;
    g->currs[0].I = 70;
    g->currs[1].x = M - 5;
    g->currs[1].y = N - 7;
    g->currs[1].I = 150;

    g->volts[0].x = M - 4;
    g->volts[0].y = 3;
    g->volts[0].U = 65;
    g->volts[1].x = 7;
    g->volts[1].y = N - 6;
    g->volts[1].U = 110;
}

int main(int argc, char **argv) {
    if (argc != 5) {
        printf("Incorrect argc count\n");
        exit(EXIT_FAILURE);
    }

    size_t threads_count = atoi(argv[1]);
    size_t tm = atoi(argv[2]);
    M = atoi(argv[3]);
    N = atoi(argv[4]);

    if (grid_inicialize(&grid) < 0) {
        exit(EXIT_FAILURE);
    }

    FILE *gnuplot = popen("gnuplot -persist", "w");
    if (gnuplot == NULL) {
        printf("gnuplot error\n");
        exit(EXIT_FAILURE);
    }

    pthread_attr_t pattr;
    pthread_attr_init(&pattr);
    pthread_attr_setscope(&pattr, PTHREAD_SCOPE_SYSTEM);
    pthread_attr_setdetachstate(&pattr, PTHREAD_CREATE_JOINABLE);
    threads = (Thread *) calloc(threads_count, sizeof(Thread));
    pthread_barrier_init(&barr, NULL, threads_count + 1);

    size_t k = M * N / threads_count;
    if (M * N % threads_count != 0) {
        k++;
    }
    for (size_t i = 0; i < threads_count; i++) {
        threads[i].start = i * k;
        if (i != threads_count - 1) {
            threads[i].end = (i + 1) * k - 1;
        } else {
            threads[threads_count - 1].end = M * N - 1;
        }

        if (pthread_create(&(threads[i].tid), &pattr, solver, (void *) &(threads[i]))) {
            printf("Creating thread error\n");
            exit(EXIT_FAILURE);
        }
    }

    is_done = 0;
    for (size_t i = 0; i < tm; i++) {
        pthread_barrier_wait(&barr);
        if (!is_done) {
            fprintf(gnuplot, "set dgrid3d %zu,%zu\n", N, M);
            fprintf(gnuplot, "set mxtics (1)\n");
            fprintf(gnuplot, "set mytics (1)\n");
            fprintf(gnuplot, "set ticslevel 0\n");
            fprintf(gnuplot, "set hidden3d\n");
            fprintf(gnuplot, "set isosample 80\n");
            fprintf(gnuplot, "set zlabel \"U\"\n");
            fprintf(gnuplot, "set ylabel \"M\"\n");
            fprintf(gnuplot, "set xlabel \"N\"\n");
            fprintf(gnuplot, "splot '-' u 1:2:3 with lines\n");

            for (size_t x = 0; x < M; x++) {
                for (size_t y = 0; y < N; y++) {
                    fprintf(gnuplot, "%zu %zu %lf\n", x, y, grid.cur[x][y]);
                }
            }
            fprintf(gnuplot, "e\n");
            fflush(gnuplot);
        }
        double **tmp = grid.prev;
        grid.prev = grid.cur;
        grid.cur = tmp;
    }
    pclose(gnuplot);
    return 0;
}