#include <fstream>
#include <iostream>
#include <cmath>

bool cmp(const std::string& ref_name, const std::string& res_name)
{
    std::ifstream ref(ref_name), res(res_name);

    double a, b;

    while (ref >> a && res >> b)
    {
        if (std::abs(a - b) > 1e-6)
        {
            std::cout << "Mismatch in " << res_name << "\n";
            return false;
        }
    }

    std::cout << res_name << " OK\n";
    return true;
}

int main()
{
    std::ifstream data("data.txt");

    std::ofstream ref_sin("ref_sin.txt");
    std::ofstream ref_sqrt("ref_sqrt.txt");
    std::ofstream ref_pow("ref_pow.txt");

    double x, y;

    while (data >> x >> y)
    {
        ref_sin  << std::sin(x) << "\n";
        ref_sqrt << std::sqrt(x) << "\n";
        ref_pow  << std::pow(x, y) << "\n";
    }

    cmp("ref_sin.txt",  "res_sin.txt");
    cmp("ref_sqrt.txt", "res_sqrt.txt");
    cmp("ref_pow.txt",  "res_pow.txt");

    return 0;
}