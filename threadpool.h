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
    void runInMainThread();
private:
    std::queue<std::unique_ptr<Task>> m_tasks;
    std::mutex m_queue_mutex;
    std::condition_variable m_cv;
    bool stop_ = false;
    std::vector<std::thread> m_threads;
};
