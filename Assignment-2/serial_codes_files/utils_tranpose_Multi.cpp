#include <math.h>
#include "utils.h"

// Problem 01
void matrix_multiplication(double **m1, double **m2, double **result, int N) {}

// Problem 02
void transpose(double **m, double **mt, int N)
{
    int i, j;
    for (i = 0; i < N; i++)
        for (j = 0; j < N; j++)
            mt[j][i] = m[i][j];
}

void transposed_matrix_multiplication(double **m1, double **m2, double **m2t, double **result, int N)
{
    int i, j, k;
    transpose(m2, m2t, N);
    for (i = 0; i < N; i++)
        for (j = 0; j < N; j++)
        {
            result[i][j] = 0;

            for (k = 0; k < N; k++)
                result[i][j] += m1[i][k] * m2t[j][k];
        }
}

// Problem 03
void block_matrix_multiplication(double **m1, double **m2, double **result, int B, int N) {}