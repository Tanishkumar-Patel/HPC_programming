#include <omp.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

void interpolation(double *mesh, Points *points)
{
    double y0 = 1.0 / dx, y1 = 1.0 / dy;
    double del = 1.0 / (NX * NY);
    int size = GRID_X * GRID_Y;

    int nthreads = omp_get_max_threads();

    double **local_mesh = (double **)malloc(nthreads * sizeof(double *));
    for (int t = 0; t < nthreads; t++)
    {
        local_mesh[t] = (double *)calloc(size, sizeof(double));
    }

    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        double *lm = local_mesh[tid];

        #pragma omp for schedule(static)
        for (int p = 0; p < NUM_Points; p++)
        {
            double X = points[p].x, Y = points[p].y;
            int xi = (X * y0), yj = (Y * y1);

            if (xi < 0) xi = 0;
            else if (xi >= GRID_X - 1) xi = GRID_X - 2;

            if (yj < 0) yj = 0;
            else if (yj >= GRID_Y - 1) yj = GRID_Y - 2;

            double lx = X * y0 - xi;
            double ly = Y * y1 - yj;

            int bse = yj * GRID_X + xi;

            lm[bse] += (1.0 - lx) * (1.0 - ly) * del;
            lm[bse + 1] += lx * (1.0 - ly) * del;
            lm[bse + GRID_X] += (1.0 - lx) * ly * del;
            lm[bse + GRID_X + 1] += lx * ly * del;
        }
    }

    #pragma omp parallel for schedule(static)
    for (int i = 0; i < size; i++)
    {
        double sum = 0.0;
        for (int t = 0; t < nthreads; t++)
        {
            sum += local_mesh[t][i];
        }
        mesh[i] += sum;
    }

    for (int t = 0; t < nthreads; t++)
    {
        free(local_mesh[t]);
    }
    free(local_mesh);
}


void save_mesh(double *mesh)
{

    FILE *fd = fopen("Mesh.out", "w");
    if (!fd)
    {
        printf("Error creating Mesh.out\n");
        exit(1);
    }
    for (int i = 0; i < GRID_Y; i++)
    {
        for (int j = 0; j < GRID_X; j++)
        {
            fprintf(fd, "%lf ", mesh[i * GRID_X + j]);
        }
        fprintf(fd, "\n");
    }

    fclose(fd);
}