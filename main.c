#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <mpi.h>

#define LEFT_EDGE_TYPE 1
#define LEFT_EDGE_VALUE 30

#define RIGHT_EDGE_TYPE 2
#define RIGHT_EDGE_VALUE 5

#define TOP_EDGE_TYPE 1
#define TOP_EDGE_VALUE 70

#define BOTTOM_EDGE_TYPE 2
#define BOTTOM_EDGE_VALUE 0

#define M 6       //длина
#define N 6      //ширина
#define START_T 0   //стартовая температура пластины
#define TIME 1000   //время расчета
#define dx 1
#define dy 1
#define dt 0.1


void write_res_to_file(int fd, double *arr) {
    for(size_t i = 0; i < M; i++) {
        for(size_t k = 0; k < N; k++) {
            dprintf(fd, "%zu %zu %lf\n", i, k, arr[i * N + k]);
        }
    }
    dprintf(fd, "\n");
}

void send_line(double *prev, double *top_line, double *bottom_line, int myrank, int total) {
    if(myrank != total - 1) {
        MPI_Send(prev + (M / total - 1) * N, N, MPI_DOUBLE, myrank + 1, 0, MPI_COMM_WORLD);
    }
    if(myrank != 0) {
        MPI_Recv(top_line, N, MPI_DOUBLE, myrank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Send(prev, N, MPI_DOUBLE, myrank - 1, 0, MPI_COMM_WORLD);
    }
    if(myrank != total - 1) {
        MPI_Recv(bottom_line, N, MPI_DOUBLE, myrank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
}

int solver(double *prev, double *curr, double *top_line, double *bottom_line, int myrank, int total) {
    for(size_t i = 0; i < M / total; i++) {
        if(i == 0 && myrank == 0) {
            continue;
        }
        if (i == M / total - 1 && myrank == total - 1) {
            break;
        }

        for(size_t k = 1; k < N - 1; k++) {
            if (i == 0) {
                curr[i * N + k] = prev[i * N + k] + dt * ((prev[i * N + k - 1] + prev[i * N + k + 1] - 2 * prev[i * N + k]) / (dx * dx) + (prev[(i + 1) * N + k] + top_line[k] - 2 * prev[i * N + k]) / (dy * dy));
            } else if (i == M / total - 1) {
                curr[i * N + k] = prev[i * N + k] + dt * ((prev[i * N + k - 1] + prev[i * N + k + 1] - 2 * prev[i * N + k]) / (dx * dx) + (bottom_line[k] + prev[(i - 1) * N + k] - 2 * prev[i * N + k]) / (dy * dy));
            } else {
                curr[i * N + k] = prev[i * N + k] + dt *
                                                    ((prev[i * N + k - 1] + prev[i * N + k + 1] - 2 * prev[i * N + k]) /
                                                     (dx * dx) + (prev[(i + 1) * N + k] + prev[(i - 1) * N + k] -
                                                                  2 * prev[i * N + k]) / (dy * dy));
            }
            if (i == 1 && myrank == 0) {
                if (TOP_EDGE_TYPE == 1) {
                    curr[k] = TOP_EDGE_VALUE;
                } else if (TOP_EDGE_TYPE == 2) {
                    curr[k] = curr[(i) * N + k] - dy * TOP_EDGE_VALUE;
                }
            }
            if (i == M / total - 2 && myrank == total - 1) {
                if (BOTTOM_EDGE_TYPE == 1) {
                    curr[(i + 1) * N + k] = BOTTOM_EDGE_VALUE;
                } else if (BOTTOM_EDGE_TYPE == 2) {
                    curr[(i + 1) * N + k] = curr[(i) * N + k] - dy * BOTTOM_EDGE_VALUE;
                }
            }
        }
        if (LEFT_EDGE_TYPE == 1) {
            curr[i * N] = LEFT_EDGE_VALUE;
        } else if (LEFT_EDGE_TYPE == 2) {
            curr[i * N] = curr[i * N + 1] - dy * LEFT_EDGE_VALUE;
        }

        if (RIGHT_EDGE_TYPE == 1) {
            curr[(i + 1) * N - 1] = RIGHT_EDGE_VALUE;
        } else if (RIGHT_EDGE_TYPE == 2) {
            curr[(i + 1) * N - 1] = curr[(i + 1) * N - 2] - dy * RIGHT_EDGE_VALUE;
        }
    }

    //углы
    if (myrank == 0) {
        curr[0] = curr[1];
        curr[N - 1] = curr[N - 2];
    }
    if (myrank == total - 1) {
        curr[(M / total - 1) * N] = curr[(M / total - 1) * N + 1];
        curr[(M / total) * N - 1] = curr[(M / total) * N - 2];
    }
    return 0;
}

int main(int argc, char **argv) {
    int myrank;
    int total;


    int result_file = open("result.txt", O_WRONLY | O_CREAT | S_IWRITE);
    if (result_file < 0) {
        exit(EXIT_FAILURE);
    }

    MPI_Init (&argc, &argv);
    MPI_Comm_size (MPI_COMM_WORLD, &total);
    MPI_Comm_rank (MPI_COMM_WORLD, &myrank);

    int len = N / total;
    double *matrix = NULL;
    if(!myrank) {
        matrix = malloc(M * N * sizeof(double));
    }

    double *curr = malloc(N * len * sizeof(double));
    double *prev = malloc(N * len * sizeof(double));
    double *top_line = malloc(N * sizeof(double));
    double *bottom_line = malloc(N * sizeof(double));

    for(size_t i = 0; i < len; i++) {
        for(size_t k = 0; k < N; k++) {
            prev[i * N + k] = START_T;
            printf("%lf ", prev[i * N + k]);
        }
        printf("\n");
    }
    printf("-----ok\n---\n");

    for (size_t i = 0; i < TIME; i++) {
        send_line(prev, top_line, bottom_line, myrank, total);
        solver(prev, curr, top_line, bottom_line, myrank, total);
        MPI_Gather(curr, len * N, MPI_DOUBLE, matrix, len * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        double *tmp = prev;
        prev = curr;
        curr = tmp;

        if(!myrank) {
            write_res_to_file(result_file, matrix);
            printf("__--___--___\n");
            for(size_t i = 0; i < M; i++) {
                for (size_t k = 0; k < N; k++) {
                    printf("%lf ", matrix[i * N + k]);
                }
                printf("\n");
            }
            printf("__--___--___\n");
        }
    }


    close(result_file);
    MPI_Finalize();
    return 0;
}
