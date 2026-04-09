#include <vector>
#include <cmath>
#include <iostream>


#ifndef USE_DOUBLE
#define USE_DOUBLE 0
#endif

#if USE_DOUBLE 
using pres = double;
#else 
using pres = float;
#endif


int main() 
{
    constexpr size_t N = 10'000'000;
    constexpr pres PI = static_cast<pres>(3.14159265358979323846);
    std::vector<pres> data(N);

    // Шаг аргумента синуса
    pres step = static_cast<pres>(2) * PI / static_cast<pres>(N);

    for (size_t i = 0; i < N; ++i) 
    {
        data[i] = std::sin(step * static_cast<pres>(i));
    }

    pres sum = 0;

    for (const auto& v : data) 
    {
        sum = sum + v;
    }

    #if USE_DOUBLE
        std::cout << "Array type: double\n";
    #else
        std::cout << "Array type: float\n";
    #endif

    std::cout << "Sum: " << sum << std::endl;

    return 0;
}