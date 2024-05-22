#include "threadpool.h"

ThreadPool::ThreadPool(size_t threadCount) {
    for (size_t i = 0; i < threadCount; ++i) {
        m_threads.emplace_back([this] {
            while (true) {
                std::unique_ptr<Task> task;
                {
                    std::unique_lock<std::mutex> lock(
                            queue_mutex_);

                    cv_.wait(lock, [this] {
                        return !tasks_.empty() || stop_;
                    });

                    if (stop_ && tasks_.empty()) {
                        return;
                    }

                    task = std::move(tasks_.front());
                    tasks_.pop();
                }

                task->Run();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        stop_ = true;
    }

    cv_.notify_all();

    for (auto &thread: m_threads) {
        thread.join();
    }
}


void ThreadPool::enqueue(std::unique_ptr<Task> &&task) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        tasks_.emplace(std::move(task));
    }
    cv_.notify_one();
}