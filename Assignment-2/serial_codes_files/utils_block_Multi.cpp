#include <math.h>
#include "utils.h"

// Problem 01
void matrix_multiplication(double **m1, double **m2, double **result, int N) {}

// Problem 02
void transpose(double **m, double **mt, int N) {}

void transposed_matrix_multiplication(double **m1, double **m2, double **result, int N) {}

// Problem 03
void block_matrix_multiplication(double **m1, double **m2, double **result, int B, int N)
{
    int i, j, k, ii, jj, kk;

    for (ii = 0; ii < N; ii += B)
    {
        for (jj = 0; jj < N; jj += B)
        {
            for (kk = 0; kk < N; kk += B)
            {
                {
                    for (i = ii; i < ii + B && i < N; i++)
                        for (j = jj; j < jj + B && j < N; j++)
                        {
                            for (k = kk; k < kk + B && k < N; k++)
                                result[i][j] += m1[i][k] * m2[k][j];
                        }
                }
            }
        }
    }
}
