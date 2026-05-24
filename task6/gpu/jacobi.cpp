#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <fstream>
#include <omp.h>
#include <iostream>
#include <boost/program_options.hpp>

#include "laplace2d.h"
#include <nvtx3/nvToolsExt.h>

namespace po = boost::program_options;

void saveMatrix(const char* fileName, double* matrix, int rows, int cols)
{
    std::ofstream outFile(fileName);

    for (int row = 0; row < rows; row++)
    {
        for (int col = 0; col < cols; col++)
        {
            outFile << matrix[row * cols + col] << " ";
        }
        outFile << "\n";
    }

    outFile.close();
}

int main(int argc, char **argv)
{
    int gridSize;
    double epsilon;
    int maxIterations;

    po::options_description desc("Options");
    desc.add_options()
        ("n", po::value<int>(&gridSize)->required(), "grid size (n x n)")
        ("eps", po::value<double>(&epsilon)->default_value(1e-6), "accuracy")
        ("iter", po::value<int>(&maxIterations)->default_value(1000000), "max iterations");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    int width = gridSize;
    double maxError = 1.0;

    double *__restrict grid = (double *)malloc(sizeof(double) * gridSize * width);
    double *__restrict gridNext = (double *)malloc(sizeof(double) * gridSize * width);

    nvtxRangePushA("init");
    initialize(grid, gridNext, width, gridSize);
    nvtxRangePop();

    int iteration = 0;

    auto startTime = std::chrono::high_resolution_clock::now();

    nvtxRangePushA("main loop");

    while (maxError > epsilon && iteration < maxIterations)
    {
        nvtxRangePushA("calc");
        maxError = calcNext(grid, gridNext, width, gridSize);
        nvtxRangePop();

        nvtxRangePushA("swap");
        double* tmp = grid;
        grid = gridNext;
        gridNext = tmp;
        nvtxRangePop();

        iteration++;
    }
    
    nvtxRangePop();

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> runtime = endTime - startTime;

    saveMatrix("result.txt", grid, gridSize, width);

    printf("Iterations: %d\n", iteration);
    printf("Time: %f s\n", runtime.count());

    deallocate(grid, gridNext);

    return 0;
}