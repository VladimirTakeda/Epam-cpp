#pragma once

#include <thread>
#include <vector>
#include <functional>
#include <condition_variable>
#include <mutex>
#include <queue>

#include "task.h"

class ThreadPool {
public:
    explicit ThreadPool(size_t threadCount = std::thread::hardware_concurrency());
    ~ThreadPool();
    void enqueue(std::unique_ptr<Task> &&task);

private:
    std::queue<std::unique_ptr<Task>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable cv_;
    bool stop_ = false;
    std::vector<std::thread> m_threads;
};
