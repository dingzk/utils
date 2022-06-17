#include <iostream>
#include "utils/threadpool.h"
#include <functional>
#include <unistd.h>

int main(void) {

    ThreadPool<std::function<void()> > pool;
    pool.Init();
    for (auto i=0 ; i< 10000; i++) {
        pool.Push(i, []() {
            std::cout << "hello world" << std::endl;
        });
    }

    sleep(20);

    pool.StatsWorkers();

    sleep(100);

    return 0;
}