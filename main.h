#ifndef LAB2_MAIN_H
#define LAB2_MAIN_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>

#define GRAPH 0

typedef struct {
    pthread_t tid;
    int start;
    int end;
} Thread;                   //потоки

typedef struct {
    double U;
    size_t x;
    size_t y;
} VoltS;                    //структура для источников напряжения

typedef struct {
    double I;
    size_t x;
    size_t y;
} CurrS;                    //структура для источников тока

typedef struct {
    double **prev;
    double **cur;
    VoltS *volts;
    CurrS *currs;
    size_t v_count;
    size_t c_count;
} Grid;                     //структура для сетки


int is_done;
size_t M;                   //длина
size_t N;                   //ширина
double R = 1;               //сопротивление резисторов
double h = 0.1;             //шаг
double C = 0.5;             //емкость конденсаторов
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
