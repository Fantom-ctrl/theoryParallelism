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
    std::ofstream fileOut(fileName);

    for (int row = 0; row < rows; row++)
    {
        for (int col = 0; col < cols; col++)
        {
            fileOut << matrix[row * cols + col] << " ";
        }
        fileOut << "\n";
    }

    fileOut.close();
}

int main(int argc, char **argv)
{
    int gridSize;
    double accuracy;
    int maxIterations;

    po::options_description optionsDesc("Options");
    optionsDesc.add_options()
        ("n", po::value<int>(&gridSize)->required(), "grid size (n x n)")
        ("eps", po::value<double>(&accuracy)->default_value(1e-6), "accuracy")
        ("iter", po::value<int>(&maxIterations)->default_value(1000000), "max iterations");

    po::variables_map variablesMap;
    po::store(po::parse_command_line(argc, argv, optionsDesc), variablesMap);
    po::notify(variablesMap);

    int gridWidth = gridSize;
    double currentError = 1.0;

    double * currentMatrix = (double *)malloc(sizeof(double) * gridSize * gridWidth);
    double * nextMatrix = (double *)malloc(sizeof(double) * gridSize * gridWidth);

    nvtxRangePushA("init");
    initialize(currentMatrix, nextMatrix, gridWidth, gridSize);
    nvtxRangePop();

    int iterationCount = 0;

    auto timeStart = std::chrono::high_resolution_clock::now();

    nvtxRangePushA("main loop");

    while (currentError > accuracy && iterationCount < maxIterations)
    {
        nvtxRangePushA("calc");
        currentError = calcNext(currentMatrix, nextMatrix, gridWidth, gridSize);
        nvtxRangePop();

        nvtxRangePushA("swap");
        double* bufferMatrix = currentMatrix;
        currentMatrix = nextMatrix;
        nextMatrix = bufferMatrix;
        nvtxRangePop();

        iterationCount++;
    }

    nvtxRangePop();

    auto timeEnd = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> executionTime = timeEnd - timeStart;

    saveMatrix("result.txt", currentMatrix, gridSize, gridWidth);

    printf("Iterations: %d\n", iterationCount);
    printf("Time: %f s\n", executionTime.count());

    deallocate(currentMatrix, nextMatrix);

    return 0;
}