#include <omp.h>
#include <iostream>
#include <memory>
#include <chrono>


int main(int argc, char *argv[]) 
{

    size_t cols = 20'000;
    size_t rows = 20'000;
    
    if (argc > 1) 
    {
        rows = atoi(argv[1]);
    }
    if (argc > 2)
    {
        cols = atoi(argv[2]);
    }

    auto matrix = std::make_unique<double[]>(rows * cols);
    auto vector = std::make_unique<double[]>(cols);
    auto result = std::make_unique<double[]>(rows);

    for (size_t i = 0; i < rows; i++) 
    {
        size_t row_offset = i * cols;
        for (size_t j = 0; j < cols; j++) 
        {
            matrix[row_offset + j] = i + j;
        }
    }

    for (size_t j = 0; j < cols; j++)
    {
        vector[j] = j;
    }


    const auto start = std::chrono::steady_clock::now();
    #pragma omp parallel
    {
        size_t threads_count = omp_get_num_threads();
        size_t thread_id = omp_get_thread_num();
        size_t rows_per_thread = rows / threads_count;
        size_t lower_bound = thread_id * rows_per_thread;
        size_t upper_bound = 0;

        if (thread_id == threads_count - 1) 
        {
            upper_bound = (rows - 1);
        }
        else
        {
            upper_bound = (lower_bound + rows_per_thread - 1);
        }

        for (size_t i = lower_bound; i <= upper_bound; i++) 
        {
            result[i] = 0.0;
            for (size_t j = 0; j < cols; j++) 
            {
                result[i] += matrix[i*cols + j] * vector[j];
            }
        }
    }
    const auto end = std::chrono::steady_clock::now();

    std::cout << std::chrono::duration<double>(end-start).count() << std::endl;
};
