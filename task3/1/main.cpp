#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include <cstdlib>

double* A = nullptr;
double* B = nullptr;
double* C = nullptr;
size_t H = 20000;
size_t W = 20000;

struct Seg
{
    size_t r1, r2;
    size_t c1, c2;
};
Seg* segs = nullptr;


void prep(Seg s)
{
    for (size_t i = s.r1; i < s.r2; ++i) 
    {
        size_t base = i * W;
        for (size_t j = 0; j < W; ++j)
        {
            A[base + j] = i + j;
        }
    }
    for (size_t j = s.c1; j < s.c2; ++j)
    {
        B[j] = j;
    }
}

void calc(Seg s)
{
    for (size_t i = s.r1; i < s.r2; ++i) 
    {
        double acc = 0.0;
        size_t base = i * W;
        for (size_t j = 0; j < W; ++j)
        {
            acc += A[base + j] * B[j];
        }
        C[i] = acc;
    }
}

int main(int argc, char** argv) 
{
    size_t T = 1;

    if (argc > 1) T = atoi(argv[1]);
    if (argc > 2) H = atoi(argv[2]);
    if (argc > 3) W = atoi(argv[3]);

    auto pA = std::make_unique<double[]>(H * W);
    auto pB = std::make_unique<double[]>(W);
    auto pC = std::make_unique<double[]>(H);

    ::A = pA.get();
    ::B = pB.get();
    ::C = pC.get();

    size_t dr = H / T;
    size_t dc = W / T;

    auto pS = std::make_unique<Seg[]>(T);
    ::segs = pS.get();

    for (size_t t = 0; t < T; ++t) 
    {
        segs[t].r1 = t * dr;
        segs[t].r2 = (t == T - 1) ? H : segs[t].r1 + dr;

        segs[t].c1 = t * dc;
        segs[t].c2 = (t == T - 1) ? W : segs[t].c1 + dc;
    }

    const auto t0 = std::chrono::steady_clock::now();

    auto th1 = std::make_unique<std::jthread[]>(T);
    for (size_t i = 0; i < T; ++i)
    {
        th1[i] = std::jthread(prep, segs[i]);
    }
    th1.reset();

    auto th2 = std::make_unique<std::jthread[]>(T);
    for (size_t i = 0; i < T; ++i)
    {
        th2[i] = std::jthread(calc, segs[i]);
    }
    th2.reset();

    const auto t1 = std::chrono::steady_clock::now();

    std::cout << std::chrono::duration<double>(t1 - t0).count() << std::endl;

    return 0;
}