#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <omp.h>
#include "utils.h"
using namespace std;

double min_val, max_val;

void interpolation(double *mesh_value, Points *points) 
{
    int size = GRID_X * GRID_Y;

    #pragma omp parallel for
    for (int i = 0; i < size; i++)
        mesh_value[i] = 0.0;
    #pragma omp parallel
    {
        double *local_mesh = (double*)calloc(size, sizeof(double));

        #pragma omp for nowait
        for (int p = 0; p < NUM_Points; p++) {
            if (points[p].is_void) continue;

            double x = points[p].x;
            if (x < 0.0) x = 0.0;
            if (x >= 1.0) x = 1.0 - 1e-10;

            double y = points[p].y;
            if (y < 0.0) y = 0.0;
            if (y >= 1.0) y = 1.0 - 1e-10;

            int i = (int)(x / dx);
            int j = (int)(y / dy);

            if (i >= GRID_X - 1) i = GRID_X - 2;
            if (j >= GRID_Y - 1) j = GRID_Y - 2;

            double Xi = i * dx;
            double Yj = j * dy;

            double lx = x - Xi;
            double ly = y - Yj;

            double w00 = (dx - lx) * (dy - ly);
            double w10 = lx * (dy - ly);
            double w01 = (dx - lx) * ly;
            double w11 = lx * ly;

            int row = j * GRID_X;
            int row_up = (j + 1) * GRID_X;

            local_mesh[row + i]         += w00;
            local_mesh[row + i + 1]     += w10;
            local_mesh[row_up + i]      += w01;
            local_mesh[row_up + i + 1]  += w11;
        }

        #pragma omp critical
        {
            for (int i = 0; i < size; i++)
                mesh_value[i] += local_mesh[i];
        }

        free(local_mesh);
    }
}

void normalization(double *mesh_value) {
    int size = GRID_X * GRID_Y;

    min_val = mesh_value[0];
    max_val = mesh_value[0];

    #pragma omp parallel for reduction(min:min_val) reduction(max:max_val)
    for (int i = 0; i < size; i++) {
        if (mesh_value[i] < min_val) min_val = mesh_value[i];
        if (mesh_value[i] > max_val) max_val = mesh_value[i];
    }

    double range = max_val - min_val;
    if (range == 0.0) return;

    #pragma omp parallel for
    for (int i = 0; i < size; i++)
        mesh_value[i] = 2.0 * (mesh_value[i] - min_val) / range - 1.0;
}

void mover(double *mesh_value, Points *points) {

    #pragma omp parallel for
    for (int p = 0; p < NUM_Points; p++) 
    {
        if (points[p].is_void) continue;

        double x = points[p].x;
        if (x < 0.0) x = 0.0;
        if (x >= 1.0) x = 1.0 - 1e-10;

        double y = points[p].y;
        if (y < 0.0) y = 0.0;
        if (y >= 1.0) y = 1.0 - 1e-10;

        int i = (int)(x / dx);
        int j = (int)(y / dy);

        if (i >= GRID_X - 1) i = GRID_X - 2;
        if (j >= GRID_Y - 1) j = GRID_Y - 2;

        double Xi = i * dx;
        double Yj = j * dy;

        double lx = x - Xi;
        double ly = y - Yj;

        double w00 = (dx - lx) * (dy - ly);
        double w10 = lx * (dy - ly);
        double w01 = (dx - lx) * ly;
        double w11 = lx * ly;

        int row = j * GRID_X;
        int row_up = (j + 1) * GRID_X;

        double Fi =
            w00 * mesh_value[row + i] +
            w10 * mesh_value[row + i + 1] +
            w01 * mesh_value[row_up + i] +
            w11 * mesh_value[row_up + i + 1];

        points[p].x += Fi * dx;
        points[p].y += Fi * dy;

        if (points[p].x < 0.0 || points[p].x > 1.0 ||
            points[p].y < 0.0 || points[p].y > 1.0)
            points[p].is_void = true;
    }
}

void denormalization(double *mesh_value) 
{
    double range = max_val - min_val;
    if (range == 0.0) return;

    int size = GRID_X * GRID_Y;
    #pragma omp parallel for
    for (int i = 0; i < size; i++)
        mesh_value[i] = (mesh_value[i] + 1.0) * 0.5 * range + min_val;
}

long long int void_count(Points *points)
{
    long long int voids = 0;
    #pragma omp parallel for reduction(+:voids)
    for (int i = 0; i < NUM_Points; i++)
        voids += (long long int)points[i].is_void;

    return voids;
}

void save_mesh(double *mesh_value)
{
    FILE *fd = fopen("Mesh.out", "w");
    if (!fd)
    {
        printf("Error creating Mesh.out\n");
        exit(1);
    }

    for (int i = 0; i < GRID_Y; i++)
    {
        int row = i * GRID_X;
        for (int j = 0; j < GRID_X; j++)
            fprintf(fd, "%lf ", mesh_value[row + j]);
        fprintf(fd, "\n");
    }
    fclose(fd);
}