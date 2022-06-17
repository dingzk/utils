#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

template <class T> class Worker {
public:
    explicit Worker(int queue_size = 1024)
        : m_queue_size(queue_size), m_count(0){};
    ~Worker() = default;

public:
    T Pop() {
        std::unique_lock<std::mutex> lock(m_mtx);
        while (m_queue.empty()) {
            m_not_full.wait(lock);
        }
        T t = std::move(m_queue.front());
        m_queue.pop();
        lock.unlock();
        m_count++;
        m_not_empty.notify_one();

        return t;
    }

    void Push(T &&ele) {
        std::unique_lock<std::mutex> lock(m_mtx);
        while (m_queue.size() > m_queue_size) {
            m_not_empty.wait(lock);
        }
        m_queue.emplace(std::forward<T>(ele));
        lock.unlock();
        m_not_full.notify_one();
    }

    void Setid(std::thread::id tid) { m_tid = tid; }

    std::thread::id Getid() { return m_tid; }

    int GetCount() { return m_count; }

    int GetQueueCount() {
        std::unique_lock<std::mutex> lock(m_mtx);
        return m_queue.size();
    }

private:
    int m_queue_size;
    std::condition_variable m_not_full;
    std::condition_variable m_not_empty;
    std::mutex m_mtx;
    std::queue<T> m_queue;
    std::thread::id m_tid;
    std::atomic<int> m_count;
};

template <class T> class ThreadPool {
public:
    explicit ThreadPool(int queue_size = 1024, unsigned worker_num = 0)
        : m_worker_num(worker_num), m_queue_size(queue_size), m_stop(false) {
        if (m_worker_num < 1) {
            m_worker_num = m_default_worker_num;
        }
    };
    ~ThreadPool() = default;

public:
    void Stop() { m_stop = true; }

    void Init() {
        std::call_once(m_once, [this]() {
            for (auto i = 0; i < m_worker_num; ++i) {
                auto worker = std::make_shared<Worker<T>>(m_queue_size);
                m_workers.emplace_back(std::move(worker));
                std::thread t([this, i]() {
                    auto worker = m_workers[i];
                    worker->Setid(std::this_thread::get_id());
                    while (!m_stop) {
                        auto func = worker->Pop();
                        std::cout << "thread num: " << i << " get" << std::endl;
                        try {
                            func();
                        } catch (const std::exception &e) {
                            std::cout << "error: " << e.what() << std::endl;
                        }
                    }
                    std::cout << "stop thread : " << i << std::endl;
                });
                t.detach();
            }
        });
    }

    // 这里可以重载，把特殊任务扔到特殊线程
    void Push(uint64_t taskId, T &&ele) {
        std::cout << "push taskid " << taskId << std::endl;
        uint64_t index = taskId % m_worker_num;
        m_workers[index]->Push(std::move(ele));
    }

    // 统计任务线程的情况
    void StatsWorkers() {
        for (auto &&item : m_workers) {
            std::cout << "thread id:" << item->Getid()
                      << " process hit:" << item->GetCount()
                      << " now queue count:" << item->GetQueueCount()
                      << std::endl;
        }
    }

private:
    unsigned m_worker_num;
#ifdef __linux
    const unsigned int m_default_worker_num = std::thread::hardware_concurrency() * 2;
#else
    const unsigned int m_default_worker_num = 2;
#endif
    int m_queue_size;
    std::atomic<bool> m_stop;
    std::vector<std::shared_ptr<Worker<T>>> m_workers;
    std::once_flag m_once;
};

#endif // THREADPOOL_H