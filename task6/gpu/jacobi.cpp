#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <fstream>
#include <omp.h>
#include <iostream>
#include <boost/program_options.hpp>

#include "laplace2d.h"

namespace po = boost::program_options;

void saveMatrix(const char* file_path, double* grid, int rows, int cols)
{
    std::ofstream out(file_path);

    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            out << grid[r * cols + c] << " ";
        }
        out << "\n";
    }

    out.close();
}

int main(int argc, char **argv)
{
    int grid_size;
    double tolerance;
    int max_iters;

    po::options_description desc("Options");
    desc.add_options()
        ("n", po::value<int>(&grid_size)->required(), "grid size (n x n)")
        ("eps", po::value<double>(&tolerance)->default_value(1e-6), "accuracy")
        ("iter", po::value<int>(&max_iters)->default_value(1000000), "max iterations");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    int cols = grid_size;
    int rows = grid_size;

    double error_val = 1.0;

    double *__restrict grid = (double *)malloc(sizeof(double) * rows * cols);
    double *__restrict grid_next = (double *)malloc(sizeof(double) * rows * cols);

    initialize(grid, grid_next, cols, rows);

    int iteration = 0;

    auto start_time = std::chrono::high_resolution_clock::now();

    while (error_val > tolerance && iteration < max_iters)
    {
        if (iteration % 100 == 0)
            error_val = calcNext(grid, grid_next, cols, rows);
        else
            calcNextNoError(grid, grid_next, cols, rows);

        double* temp = grid;
        grid = grid_next;
        grid_next = temp;

        iteration++;
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> runtime = end_time - start_time;

    #pragma acc update self(grid[:rows*cols])
    saveMatrix("result.txt", grid, rows, cols);

    printf("Iterations: %d\n", iteration);
    printf("Time: %f s\n", runtime.count());

    deallocate(grid, grid_next);

    return 0;
}