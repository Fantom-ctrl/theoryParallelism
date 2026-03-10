#include <omp.h>
#include <iostream>
#include <memory>
#include <cmath>
#include <chrono>


int main(int argc, char* argv[]) 
{

    size_t n = 5'000;

    if (argc > 1) 
    {
        n = atoi(argv[1]);
    }

    auto A = std::make_unique<double[]>(n*n);
    auto B = std::make_unique<double[]>(n);
    auto X = std::make_unique<double[]>(n);
    auto Y = std::make_unique<double[]>(n);

    for (size_t i = 0; i < n; i++) 
    {
        for (size_t j = 0; j < n; j++) 
        {
            if (i==j)
            {
                A[i*n + j] = 2.0;
            }
            else
            {
                A[i*n + j] = 1.0;
            }
        }
        X[i] = 0.0;
        B[i] = n + 1;
    }

    double e = 1e-6;
    const auto s = std::chrono::steady_clock::now();

    size_t it = 1000;
    for (size_t k = 0; k < it; k++) 
    {
        #pragma omp parallel for
        for (size_t i = 0; i < n; i++) 
        {
            double t = 0.0;
            size_t r = i*n;
            for (size_t j = 0; j < n; j++) 
            {
                if (i != j)
                {
                    t += A[r + j] * X[j];
                }
            }
            Y[i] = (B[i] - t) / A[r + i];    
        }
        
        double m = 0.0;
        #pragma omp parallel for reduction(max:m)
        for (size_t i = 0; i < n; i++) 
        {
            double d = std::fabs(Y[i] - X[i]);
            if (d > m)
            {
                m = d;
            }
        }

        if (m < e) 
        {
            break;
        }

        #pragma omp parallel for
        for (size_t i = 0; i < n; i++) 
        {
            X[i] = Y[i];
        }
    }

    const auto f = std::chrono::steady_clock::now();

    std::cout << std::chrono::duration<double>(f-s).count() << std::endl;
};
