#ifndef LAB2_MAIN_H
#define LAB2_MAIN_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>

typedef struct {
    pthread_t tid;
    int start;
    int end;
} Thread;

typedef struct {
    double U;
    size_t x;
    size_t y;
} VoltS;

typedef struct {
    double I;
    size_t x;
    size_t y;
} CurrS;

typedef struct {
    double **prev;
    double **cur;
    VoltS *volts;
    CurrS *currs;
    size_t v_count;
    size_t c_count;
} Grid;


int is_done;
size_t M;
size_t N;
double R = 1;
double h = 0.1;
double C = 0.5;
Grid grid;

Thread *threads;
pthread_barrier_t barr;

int is_ok_knot(ssize_t m, ssize_t n);
int volts_solver(size_t m, size_t n);
int currs_solver(ssize_t m, ssize_t n);
void point_solver(size_t i);
void *solver(void *arg);
int grid_inicialize(Grid *g);
int main(int argc, char **argv);
#endif //LAB2_MAIN_H
