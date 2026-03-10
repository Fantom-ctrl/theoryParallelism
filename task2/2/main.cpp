#include <stdio.h>
#include <iostream>
#include <omp.h>
#include <fstream>
#include <chrono>
#include <cmath>


double gaussian(double x) 
{
    return exp(-x*x);
};

int main(int argc, char* argv[]) 
{

    size_t steps_count = 40'000'000;

    if (argc > 1) 
    {
        steps_count = atoi(argv[1]);
    }

    double left_bound = -4.0;
    double right_bound = 4.0;
    double step_size = (right_bound - left_bound) / steps_count;
    double integral_sum = 0.0;

    auto time_start = std::chrono::steady_clock::now();
  
    #pragma omp parallel for reduction(+:integral_sum) schedule(static)
    for (int step_index = 0; step_index < steps_count; step_index++) 
    {
        integral_sum += gaussian(left_bound + step_size * (step_index + 0.5));
    }
    integral_sum *= step_size;

    auto time_end = std::chrono::steady_clock::now();

    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(time_end - time_start).count() << std::endl;
};
