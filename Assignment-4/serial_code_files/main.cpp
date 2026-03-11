// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <time.h>
// #include <omp.h>
// #include <bits/stdc++.h>
// #include "init.h"
// #include "utils.h"
// using namespace std;

// // Global variables

// int GRID_X, GRID_Y, NX, NY;
// int NUM_Points, Maxiter;
// double dx, dy;

// int main(int argc, char **argv) {

//     // Fixed Parameters
//     NX = 1000;
//     NY = 400;
//     Maxiter = 10;
//     // NUM_Points = 100000000;
//     vector<double> points_range={1e2,1e4,1e6,1e8};

//     // Since Number of points will be 1 more than number of cells
//     GRID_X = NX + 1;
//     GRID_Y = NY + 1;
//     dx = 1.0 / NX;
//     dy = 1.0 / NY;

//     // Fix Number of Threads
//     omp_set_num_threads(4);

//     // Allocate memory for grid and Points
    
//     for(int i=0;i<points_range.size();i++)
//     {
//         cout<<"POINTS: "<<points_range[i]<<endl;
//         cout<<endl;
//         NUM_Points = points_range[i];
//         double *mesh_value = (double *) calloc(GRID_X * GRID_Y, sizeof(double));
//         Points *points = (Points *) calloc(points_range[i], sizeof(Points));

//         initializepoints(points);
//             // printf("Iter\tInterp\t\tMover\t\tTotal\n");
//             printf("Iter\tInterp\n");

//             for (int iter = 0; iter < Maxiter; iter++) {

//                 // Interpolation timing
//                 clock_t start_interp = clock();
//                 interpolation(mesh_value, points);
//                 clock_t end_interp = clock();

//                 // Mover timing
//                 // clock_t start_move = clock();
//                 // mover_serial(points, dx, dy);
//                 // clock_t end_move = clock();

//                 double interp_time = (double)(end_interp - start_interp) / CLOCKS_PER_SEC;
//                 // double move_time = (double)(end_move - start_move) / CLOCKS_PER_SEC;
//                 // double total = interp_time + move_time;

//                 // printf("%d\t%lf\t%lf\t%lf\n", iter+1, interp_time, move_time, total);
//                 printf("%d\t%lf\t\n", iter+1, interp_time);

//             }
//             // Free memory
//             free(mesh_value);
//             free(points);
//             cout<<endl;
//     }
//     return 0;
// }
















// #include <stdio.h>
// #include <stdlib.h>
// #include <time.h>
// #include <omp.h>
// #include "init.h"
// #include "utils.h"
// using namespace std;

// // Global variables
// int GRID_X, GRID_Y, NX, NY;
// int NUM_Points, Maxiter;
// double dx, dy;

// int main(int argc, char **argv) {

//     // Fixed Parameters
//     NX = 250;
//     NY = 100;
//     Maxiter = 10;
//     NUM_Points = 100000000L; // Fixed number of particles

//     // Grid parameters
//     GRID_X = NX + 1;
//     GRID_Y = NY + 1;
//     dx = 1.0 / NX;
//     dy = 1.0 / NY;

//     // Fix Number of Threads
//     omp_set_num_threads(4);

//     // Allocate memory for grid and points
//     double *mesh_value = (double *) calloc(GRID_X * GRID_Y, sizeof(double));
//     Points *points = (Points *) calloc(NUM_Points, sizeof(Points));

//     // Initialize particles only once
//     initializepoints(points);

//     printf("Iter\tInterp\n");

//     double total_interp_time = 0.0;

//     for (int iter = 0; iter < Maxiter; iter++) {

//         // Interpolation timing
//         clock_t start_interp = clock();
//         interpolation(mesh_value, points);
//         clock_t end_interp = clock();

//         double interp_time = (double)(end_interp - start_interp) / CLOCKS_PER_SEC;
//         total_interp_time += interp_time;

//         // Print per iteration (same as before)
//         printf("%d\t%lf\n", iter + 1, interp_time);
//     }

//     // Print total interpolation time
//     printf("\nTotal Interpolation Time: %lf seconds\n", total_interp_time);

//     // Free memory
//     free(mesh_value);
//     free(points);

//     return 0;
// }


// #include <stdio.h>
// #include <stdlib.h>
// #include <time.h>
// #include <omp.h>
// #include "init.h"
// #include "utils.h"
// using namespace std;

// // Global variables
// int GRID_X, GRID_Y, NX, NY;
// int NUM_Points, Maxiter;
// double dx, dy;

// // Function prototypes
// void mover_serial(Points *points, double dx, double dy);
// void mover_parallel(Points *points, double dx, double dy);

// int main(int argc, char **argv) {

//     // Fixed Parameters
//     NX = 1000;
//     NY = 400;
//     Maxiter = 10;
//     NUM_Points = 14000000; // 14 million points

//     // Grid parameters
//     GRID_X = NX + 1;
//     GRID_Y = NY + 1;
//     dx = 1.0 / NX;
//     dy = 1.0 / NY;

//     // Fix Number of Threads
//     omp_set_num_threads(4);

//     // Allocate memory for grid and points
//     double *mesh_value = (double *) calloc(GRID_X * GRID_Y, sizeof(double));
//     Points *points = (Points *) calloc(NUM_Points, sizeof(Points));

//     // Initialize particles only once
//     initializepoints(points);

//     printf("Iter\tInterp\tMoverSerial\tMoverParallel\tTotalSerial\tTotalParallel\n");

//     double total_interp_time = 0.0;
//     double total_mover_serial_time = 0.0;
//     double total_mover_parallel_time = 0.0;

//     for (int iter = 0; iter < Maxiter; iter++) {

//         // ------------------- Interpolation -------------------
//         clock_t start_interp = clock();
//         interpolation(mesh_value, points);
//         clock_t end_interp = clock();
//         double interp_time = (double)(end_interp - start_interp) / CLOCKS_PER_SEC;
//         total_interp_time += interp_time;

//         // ------------------- Mover Serial -------------------
//         clock_t start_mover_serial = clock();
//         mover_serial(points, dx, dy);
//         clock_t end_mover_serial = clock();
//         double mover_serial_time = (double)(end_mover_serial - start_mover_serial) / CLOCKS_PER_SEC;
//         total_mover_serial_time += mover_serial_time;

//         // ------------------- Mover Parallel -------------------
//         clock_t start_mover_parallel = clock();
//         mover_parallel(points, dx, dy);
//         clock_t end_mover_parallel = clock();
//         double mover_parallel_time = (double)(end_mover_parallel - start_mover_parallel) / CLOCKS_PER_SEC;
//         total_mover_parallel_time += mover_parallel_time;

//         // ------------------- Totals per iteration -------------------
//         double total_serial = interp_time + mover_serial_time;
//         double total_parallel = interp_time + mover_parallel_time;

//         printf("%d\t%lf\t%lf\t%lf\t%lf\t%lf\n", iter + 1,
//                interp_time, mover_serial_time, mover_parallel_time,
//                total_serial, total_parallel);
//     }

//     // ------------------- Print cumulative totals -------------------
//     printf("\nTotal Interpolation Time: %lf seconds\n", total_interp_time);
//     printf("Total Mover Serial Time: %lf seconds\n", total_mover_serial_time);
//     printf("Total Mover Parallel Time: %lf seconds\n", total_mover_parallel_time);
//     printf("Total Simulation Time Serial: %lf seconds\n", total_interp_time + total_mover_serial_time);
//     printf("Total Simulation Time Parallel: %lf seconds\n", total_interp_time + total_mover_parallel_time);

//     // Free memory
//     free(mesh_value);
//     free(points);

//     return 0;
// }
