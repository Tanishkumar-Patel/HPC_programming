#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

// Serial interpolation
void interpolation(double *mesh, Points *points)
{
    double y0 = 1.0 / dx, y1 = 1.0 / dy;
    double del = 1.0 / (NX * NY);
    for (int p = 0; p < NUM_Points; p++)
    {
        double X = points[p].x, Y = points[p].y;
        int xi = (X * y0), yj = (Y * y1);

        if (xi < 0)
            xi = 0;
        else if (xi >= GRID_X - 1)
            xi = GRID_X - 2;

        if (yj < 0)
            yj = 0;
        else if (yj >= GRID_Y - 1)
            yj = GRID_Y - 2;

        double lx = X * y0 - xi, ly = Y * y1 - yj;
        int bse = yj * GRID_X + xi;
        mesh[bse] += (1.0 - lx) * (1.0 - ly) * del;
        mesh[bse + 1] += lx * (1.0 - ly) * del;
        mesh[bse + GRID_X] += (1.0 - lx) * ly * del;
        mesh[bse + GRID_X + 1] += lx * ly * del;
    }
}

// Write mesh to file
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