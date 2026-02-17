#include <math.h>
#include "utils.h"

// Problem 01
void matrix_multiplication(double **m1, double **m2, double **result, int N)
{
    int i, j, k;

    for (i = 0; i < N; i++)
        for (j = 0; j < N; j++)
        {
            result[i][j] = 0;
            for (k = 0; k < N; k++)
                result[i][j] += m1[i][k] * m2[k][j];
        }
}

// Problem 02
void transpose(double **m, double **mt, int N) {}

void transposed_matrix_multiplication(double **m1, double **m2, double **result, int N) {}

// Problem 03
void block_matrix_multiplication(double **m1, double **m2, double **result, int B, int N) {}