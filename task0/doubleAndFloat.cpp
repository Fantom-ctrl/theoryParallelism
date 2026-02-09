#include <vector>
#include <cmath>
#include <iostream>

using namespace std; 

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
    vector<pres> data(N);

    // Шаг аргумента синуса
    pres step = static_cast<pres>(2) * PI / static_cast<pres>(N);

    for (size_t i = 0; i < N; ++i) 
    {
        data[i] = sin(step * static_cast<pres>(i));
    }

    pres sum = 0;

    for (const auto& v : data) 
    {
        sum = sum + v;
    }

    #if USE_DOUBLE
    cout << "Array type: double\n";
    #else
        cout << "Array type: float\n";
    #endif

    cout << "Sum: " << sum << endl;

    return 0;
}