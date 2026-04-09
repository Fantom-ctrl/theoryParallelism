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


template<typename T>
class Srv
{
private:
    std::thread th;
    std::atomic<bool> stop{false};

    std::mutex m;
    std::condition_variable cv;

    std::atomic<size_t> id_gen{0};

    std::queue<std::pair<size_t, std::packaged_task<T()>>> q;
    std::unordered_map<size_t, std::future<T>> res;

public:
    Srv() 
    {
        run();
    }

    void run()
    {
        stop.store(false);

        th = std::thread([this]()
        {
            while (true)
            {
                std::pair<size_t, std::packaged_task<T()>> cur;

                {
                    std::unique_lock<std::mutex> lk(m);

                    cv.wait(lk, [this]
                    {
                        return stop.load() || !q.empty();
                    });

                    if (stop.load() && q.empty())
                        break;

                    cur = std::move(q.front());
                    q.pop();
                }

                cur.second(); 
            }
        });
    }

    void halt()
    {
        stop.store(true);
        cv.notify_all();

        if (th.joinable())
            th.join();
    }

    template<typename F, typename... A>
    size_t push(F&& f, A&&... a)
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

    T get(size_t id)
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
        halt();
    }
};

thread_local std::mt19937 rng(std::random_device{}());
thread_local std::uniform_real_distribution<double> d(1.0, 5.0);
thread_local std::uniform_int_distribution<size_t> n_dist(6, 9999);

template<typename T>
struct arity;

template<typename R, typename... A>
struct arity<R(*)(A...)>
{
    static constexpr size_t val = sizeof...(A);
};

template<typename F, typename T, std::size_t... I>
auto call_vec(F f, const std::vector<T>& v, std::index_sequence<I...>)
{
    return f(v[I]...);
}

template<typename T, typename F>
void cli(Srv<T>& srv, F f, const std::string& name)
{
    size_t N = n_dist(rng);

    std::ofstream out(name);
    if (!out.is_open())
    {
        std::cerr << "file error\n";
        return;
    }

    constexpr size_t k = arity<std::decay_t<F>>::val;
    constexpr auto idx = std::make_index_sequence<k>{};

    std::vector<size_t> ids;

    for (size_t i = 0; i < N; i++)
    {
        std::vector<T> v;
        for (size_t j = 0; j < k; j++)
        {
            v.push_back(static_cast<T>(d(rng)));
        }

        auto task = [f, v, idx]() -> T
        {
            return call_vec(f, v, idx);
        };

        ids.push_back(srv.push(task));
    }

    for (size_t id : ids)
    {
        T r = srv.get(id);
        out << r << "\n";
    }
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

void check(const std::string& name)
{
    std::ifstream in(name);
    double x;

    while (in >> x)
    {
        if (std::isnan(x) || std::isinf(x))
        {
            std::cout << "bad " << name << "\n";
            return;
        }
    }

    std::cout << name << " OK\n";
}

int main()
{
    Srv<double> srv;

    std::thread a([&]() { cli(srv, f_sin<double>,  "res_sin.txt"); });
    std::thread b([&]() { cli(srv, f_sqrt<double>, "res_sqrt.txt"); });
    std::thread c([&]() { cli(srv, f_pow<double>,  "res_pow.txt"); });

    a.join();
    b.join();
    c.join();

    check("res_sin.txt");
    check("res_sqrt.txt");
    check("res_pow.txt");

    return 0;
}