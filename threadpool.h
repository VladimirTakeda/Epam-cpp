#pragma once

#include <thread>
#include <vector>
#include <functional>
#include <condition_variable>
#include <mutex>
#include <queue>

#include "task.h"

/// A ThreadPool with side threads and main thread to execute tasks
/// Not thread-safe (Use only with main thread)
class ThreadPool {
public:
    /// @brief creates a side threadCount of threads
    explicit ThreadPool(size_t threadCount = std::thread::hardware_concurrency());
    /// @brief invoke runInMainThread and then waiting for side threads has been finished
    ~ThreadPool();
    /// @brief add a new task to queue
    void enqueue(std::unique_ptr<Task> &&task);
private:
    /// @brief after all the tasks has been added - executes the tasks in main thread as well
    void runInMainThread();
private:
    std::queue<std::unique_ptr<Task>> m_tasks;
    std::mutex m_queue_mutex;
    std::condition_variable m_cv;
    bool stop_ = false;
    std::vector<std::thread> m_threads;
};
