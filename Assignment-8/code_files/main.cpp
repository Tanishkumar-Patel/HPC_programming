#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <omp.h>

#include "init.h"
#include "utils.h"

// Global variables
int GRID_X, GRID_Y, NX, NY;
int NUM_Points, Maxiter;
double dx, dy;

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 3)
    {
        if (rank == 0)
            printf("Usage: %s <input_file> <threads>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    int threads = atoi(argv[2]);
    omp_set_num_threads(threads);

    FILE *file = fopen(argv[1], "rb");
    if (!file)
    {
        if (rank == 0)
            printf("Error opening input file\n");
        MPI_Finalize();
        exit(1);
    }

    fread(&NX, sizeof(int), 1, file);
    fread(&NY, sizeof(int), 1, file);
    fread(&NUM_Points, sizeof(int), 1, file);
    fread(&Maxiter, sizeof(int), 1, file);

    GRID_X = NX + 1;
    GRID_Y = NY + 1;
    dx = 1.0 / NX;
    dy = 1.0 / NY;

    double *mesh_value = (double *)calloc(GRID_X * GRID_Y, sizeof(double));
    double *local_mesh = (double *)calloc(GRID_X * GRID_Y, sizeof(double));

    Points *points = (Points *)calloc(NUM_Points, sizeof(Points));
    read_points(file, points);
    fclose(file);

    // MPI partition
    int start = rank * (NUM_Points / size);
    int end = (rank == size - 1) ? NUM_Points : start + (NUM_Points / size);

    double total_int_time = 0.0;
    double total_norm_time = 0.0;
    double total_move_time = 0.0;
    double total_denorm_time = 0.0;

    for (int iter = 0; iter < Maxiter; iter++)
    {
        memset(local_mesh, 0, GRID_X * GRID_Y * sizeof(double));

        double t0 = MPI_Wtime();

        interpolation(local_mesh, points, start, end);

        MPI_Allreduce(local_mesh, mesh_value,
                      GRID_X * GRID_Y,
                      MPI_DOUBLE, MPI_SUM,
                      MPI_COMM_WORLD);

        double t1 = MPI_Wtime();

        normalization(mesh_value);

        double t2 = MPI_Wtime();

        mover(mesh_value, points, start, end);

        double t3 = MPI_Wtime();

        denormalization(mesh_value);

        double t4 = MPI_Wtime();

        total_int_time += (t1 - t0);
        total_norm_time += (t2 - t1);
        total_move_time += (t3 - t2);
        total_denorm_time += (t4 - t3);
    }

    if (rank == 0)
    {
        printf("\n===== Hybrid MPI + OpenMP =====\n");
        printf("Total Interpolation Time = %lf seconds\n", total_int_time);
        printf("Total Normalization Time = %lf seconds\n", total_norm_time);
        printf("Total Mover Time = %lf seconds\n", total_move_time);
        printf("Total Denormalization Time = %lf seconds\n", total_denorm_time);
        printf("Total Algorithm Time = %lf seconds\n",
               total_int_time + total_norm_time + total_move_time + total_denorm_time);

        printf("Total Number of Voids = %lld\n", void_count(points));

        save_mesh(mesh_value);
    }

    free(mesh_value);
    free(local_mesh);
    free(points);

    MPI_Finalize();
    return 0;
}
