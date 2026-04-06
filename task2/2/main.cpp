#include <stdio.h>
#include <iostream>
#include <omp.h>
#include <fstream>
#include <chrono>
#include <cmath>

double compute(double val) 
{
    return exp(-val*val);
};

int main(int argc, char* argv[]) 
{

    size_t steps = 40'000'000;

    if (argc > 1) 
    {
        steps = atoi(argv[1]);
    }

    double left = -4.0;
    double right = 4.0;
    double dx = (right - left) / steps;
    double total = 0.0;
  
    auto start_time = std::chrono::steady_clock::now();
    #pragma omp parallel reduction(+:total)
    {
        size_t threads_count = omp_get_num_threads();
        size_t thread_id = omp_get_thread_num();
        size_t chunk_size = steps / threads_count;
        size_t lower_bound = thread_id * chunk_size;
        size_t upper_bound = 0;
        if (thread_id == threads_count - 1)
        {
            upper_bound = steps;
        }
        else
        {
            upper_bound = lower_bound + chunk_size;
        }

        double current_x = left + dx * (lower_bound + 0.5);
        for (size_t i = lower_bound; i < upper_bound; i++)
        {   
            total += compute(current_x);
            current_x += dx;
        }
    }
    total *= dx;

    auto end_time = std::chrono::steady_clock::now();

    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() << std::endl;
};