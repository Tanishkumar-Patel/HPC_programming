#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <omp.h>

#include "utils.h"

using namespace std;

void interpolation(double *mesh_value, Points *points)
{
    memset(mesh_value, 0, (size_t)GRID_X * GRID_Y * sizeof(double));

    for (int p = 0; p < NUM_Points; p++)
    {
        double x = points[p].x;
        double y = points[p].y;

        int i = (int)(x / dx);
        int j = (int)(y / dy);

        if (i >= NX)
            i = NX - 1;
        if (j >= NY)
            j = NY - 1;
        if (i < 0)
            i = 0;
        if (j < 0)
            j = 0;

        double wx = (x - (double)i * dx) / dx;
        double wy = (y - (double)j * dy) / dy;

        mesh_value[j * GRID_X + i] += (1.0 - wx) * (1.0 - wy);
        mesh_value[j * GRID_X + i + 1] += wx * (1.0 - wy);
        mesh_value[(j + 1) * GRID_X + i] += (1.0 - wx) * wy;
        mesh_value[(j + 1) * GRID_X + i + 1] += wx * wy;
    }
}

void mover_deferred_serial(Points *points, double deltaX, double deltaY)
{
    vector<int> deleted_points;
    const double inverse_rand = 1.0 / RAND_MAX;
    for (int p = 0; p < NUM_Points; p++)
    {
        double rx = (rand() * inverse_rand * 2.0 - 1.0) * deltaX;
        double ry = (rand() * inverse_rand * 2.0 - 1.0) * deltaY;

        // move
        points[p].x += rx;
        points[p].y += ry;

        // baar jaay to
        if (points[p].x < 0.0 || points[p].x > 1.0 || points[p].y < 0.0 || points[p].y > 1.0)
        {
            deleted_points.push_back(p);
        }
    }

    for (int k = 0; k < (int)deleted_points.size(); k++)
    {
        int idx = deleted_points[k];
        points[idx].x = rand() * inverse_rand;
        points[idx].y = rand() * inverse_rand;
    }
}

unsigned int hlp1(unsigned int &state)
{
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

double grand(unsigned int &state)
{
    return hlp1(state) * (1.0 / 4294967296.0);
}

void mover_deferred_parallel(Points *points, double deltaX, double deltaY)
{
    int num_threads = 1;

#pragma omp parallel
    {
#pragma omp single
        num_threads = omp_get_num_threads();
    }

    vector<vector<int>> tvoids(num_threads);

    int per_cap = min((NUM_Points / 25 / num_threads) + 1024, 500000);
    for (int t = 0; t < num_threads; t++)
        tvoids[t].reserve(per_cap);

#pragma omp parallel
    {
        int tid = omp_get_thread_num();
        unsigned int seed = (unsigned int)(tid + 1) * 2654435761u;

#pragma omp for schedule(static)
        for (int p = 0; p < NUM_Points; p++)
        {
            double rx = (grand(seed) * 2.0 - 1.0) * deltaX;
            double ry = (grand(seed) * 2.0 - 1.0) * deltaY;

            points[p].x += rx;
            points[p].y += ry;

            if (points[p].x < 0.0 || points[p].x > 1.0 ||
                points[p].y < 0.0 || points[p].y > 1.0)
            {
                tvoids[tid].push_back(p);
            }
        }
    }

#pragma omp parallel for schedule(static)
    for (int t = 0; t < num_threads; t++)
    {
        unsigned int seed = (unsigned int)(t + 12345) * 9781u;
        for (int idx : tvoids[t])
        {
            points[idx].x = grand(seed);
            points[idx].y = grand(seed);
        }
    }
}

void mover_simple_parallel(Points *points, double deltaX, double deltaY)
{
#pragma omp parallel
    {
        int tid = omp_get_thread_num();
        unsigned int seed = (unsigned int)(tid + 1) * 362436069u;

#pragma omp for schedule(static)
        for (int p = 0; p < NUM_Points; p++)
        {
            while (1)
            {
                double rx = (grand(seed) * 2.0 - 1.0) * deltaX;
                double ry = (grand(seed) * 2.0 - 1.0) * deltaY;

                double nx = points[p].x + rx;
                double ny = points[p].y + ry;

                if (nx >= 0.0 && nx <= 1.0 &&
                    ny >= 0.0 && ny <= 1.0)
                {
                    points[p].x = nx;
                    points[p].y = ny;
                    break;
                }
            }
        }
    }
}

void save_mesh(double *mesh_value)
{
    FILE *fd = fopen("Mesh.out", "w");
    if (!fd)
    {
        fprintf(stderr, "Error creating Mesh.out\n");
        exit(1);
    }
    for (int i = 0; i < GRID_Y; i++)
    {
        for (int j = 0; j < GRID_X; j++)
        {
            fprintf(fd, "%lf ", mesh_value[i * GRID_X + j]);
        }
        fprintf(fd, "\n");
    }
    fclose(fd);
}
