#ifndef UTILS_H
#define UTILS_H

#include "init.h"

extern int GRID_X, GRID_Y, NUM_Points;
extern double dx, dy;

void interpolation(double *mesh_value, Points *points, int start, int end);
void normalization(double *mesh_value);
void mover(double *mesh_value, Points *points, int start, int end);
void denormalization(double *mesh_value);
long long int void_count(Points *points);
void save_mesh(double *mesh_value);

#endif
