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

void saveMatrix(const char* filename, double* A, int n, int m)
{
    std::ofstream out(filename);

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            out << A[i * m + j] << " ";
        }
        out << "\n";
    }

    out.close();
}

int main(int argc, char **argv)
{
    int n;
    double tol;
    int iter_max;

    po::options_description desc("Options");
    desc.add_options()
        ("n", po::value<int>(&n)->required(), "grid size (n x n)")
        ("eps", po::value<double>(&tol)->default_value(1e-6), "accuracy")
        ("iter", po::value<int>(&iter_max)->default_value(1000000), "max iterations");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    int m = n;
    double error = 1.0;

    double * A = (double *)malloc(sizeof(double) * n * m);
    double * Anew = (double *)malloc(sizeof(double) * n * m);

    nvtxRangePushA("init");
    initialize(A, Anew, m, n);
    nvtxRangePop();

    int iter = 0;

    auto start = std::chrono::high_resolution_clock::now();

    nvtxRangePushA("main loop");

    while (error > tol && iter < iter_max)
    {
        nvtxRangePushA("calc");
        error = calcNext(A, Anew, m, n);
        nvtxRangePop();

        nvtxRangePushA("swap");
        double* tmp = A;
        A = Anew;
        Anew = tmp;
        nvtxRangePop();

        iter++;
    }

    nvtxRangePop();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> runtime = end - start;

    saveMatrix("result.txt", A, n, m);

    printf("Iterations: %d\n", iter);
    printf("Time: %f s\n", runtime.count());

    deallocate(A, Anew);

    return 0;
}