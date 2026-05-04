#include <iostream>
#include <queue>
#include <future>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <functional>
#include <atomic>
#include <stdexcept>
#include <cmath>
#include <utility>
#include <fstream>
#include <random>
#include <string>
#include <type_traits>
#include <vector>
#include <chrono>
#include <argparse/argparse.hpp>


template<typename T>
class Srv
{
private:
    std::thread th;
    std::atomic<bool> s{false};

    std::mutex m;
    std::condition_variable cv;

    std::atomic<size_t> id_gen{0};

    std::queue<std::pair<size_t, std::packaged_task<T()>>> q;
    std::unordered_map<size_t, std::future<T>> res;

public:
    Srv()
    {
        start();
    }

    void start()
    {
        s.store(false);

        th = std::thread([this]()
        {
            while (true)
            {
                std::pair<size_t, std::packaged_task<T()>> cur;

                {
                    std::unique_lock<std::mutex> lk(m);

                    cv.wait(lk, [this]
                    {
                        return s.load() || !q.empty();
                    });

                    if (s.load() && q.empty())
                        break;

                    cur = std::move(q.front());
                    q.pop();
                }

                cur.second();
            }
        });
    }

    void stop()
    {
        s.store(true);
        cv.notify_all();

        if (th.joinable())
            th.join();
    }

    template<typename F, typename... A>
    size_t add_task(F&& f, A&&... a)
    {
        auto task = std::packaged_task<T()>(
            std::bind(std::forward<F>(f), std::forward<A>(a)...)
        );

        std::future<T> fut = task.get_future();

        size_t id = id_gen.fetch_add(1);

        {
            std::unique_lock<std::mutex> lk(m);
            q.push({id, std::move(task)});
            res[id] = std::move(fut);
        }

        cv.notify_one();
        return id;
    }

    T request_result(size_t id)
    {
        std::future<T> fut;

        {
            std::unique_lock<std::mutex> lk(m);

            auto it = res.find(id);
            if (it == res.end())
                throw std::runtime_error("no result");

            fut = std::move(it->second);
            res.erase(it);
        }

        return fut.get();
    }

    ~Srv()
    {
        stop();
    }
};

template<typename F, typename T>
auto call(F f, const std::vector<T>& v)
{
    if constexpr (std::is_invocable_v<F, T>)
        return f(v[0]);
    else
        return f(v[0], v[1]);
}

template<typename T, typename F>
void client(Srv<T>& srv, F f, const std::string& name, const std::vector<std::vector<T>>& data)
{
    std::ofstream out(name);
    std::vector<size_t> ids;

    for (const auto& v : data)
    {
        auto task = [f, v]() -> T
        {
            return call(f, v);
        };

        ids.push_back(srv.add_task(task));
    }

    for (size_t id : ids)
        out << srv.request_result(id) << "\n";
}

template<typename T>
T f_sin(T x)
{
    return std::sin(x);
}

template<typename T>
T f_sqrt(T x)
{
    return std::sqrt(x);
}

template<typename T>
T f_pow(T x, T y)
{
    return std::pow(x, y);
}

int main(int argc, char* argv[])
{
    argparse::ArgumentParser program("program_name");

    program.add_argument("number")
        .help("number of tasks")
        .scan<'i', int>()
        .default_value(10000);

    program.parse_args(argc, argv);

    auto N = program.get<int>("number");

    std::ofstream dataf("data.txt");
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> d(1.0, 5.0);

    for (size_t i = 0; i < N; i++)
    {
        dataf << d(rng) << " " << d(rng) << "\n";
    }

    dataf.close();

    std::vector<std::vector<double>> data;
    std::ifstream in("data.txt");

    double x, y;
    while (in >> x >> y)
        data.push_back({x, y});

    auto start = std::chrono::high_resolution_clock::now();

    Srv<double> srv;

    std::thread a([&]() { client(srv, f_sin<double>,  "res_sin.txt", data); });
    std::thread b([&]() { client(srv, f_sqrt<double>, "res_sqrt.txt", data); });
    std::thread c([&]() { client(srv, f_pow<double>,  "res_pow.txt", data); });

    a.join();
    b.join();
    c.join();

    srv.stop();

    auto end = std::chrono::high_resolution_clock::now();

    double seconds = std::chrono::duration<double>(end - start).count();
    std::cout << seconds << " seconds\n";

    return 0;
}