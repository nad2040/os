#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

void proc(void)
{
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(5s);
}

int main()
{
    std::vector<std::thread> threads;
    for (unsigned int i = 0; i < std::thread::hardware_concurrency(); ++i)
    {
        threads.emplace_back();
    }

    for (auto& t : threads)
        t.join();
}

