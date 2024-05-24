#include "threadpool.h"

ThreadPool::ThreadPool(size_t threadCount) {
    for (size_t i = 0; i < threadCount; ++i) {
        m_threads.emplace_back([this] {
            while (true) {
                std::unique_ptr<Task> task;
                {
                    std::unique_lock<std::mutex> lock(
                            m_queue_mutex);

                    m_cv.wait(lock, [this] {
                        return !m_tasks.empty() || stop_;
                    });

                    if (stop_ && m_tasks.empty()) {
                        return;
                    }

                    task = std::move(m_tasks.front());
                    m_tasks.pop();
                }

                task->Run();
            }
        });
    }
}

void ThreadPool::runInMainThread() {
    while (true) {
        std::unique_ptr<Task> task;
        {
            std::unique_lock<std::mutex> lock(
                    m_queue_mutex);

            if (m_tasks.empty()) {
                return;
            }

            task = std::move(m_tasks.front());
            m_tasks.pop();
        }

        task->Run();
    }
}

ThreadPool::~ThreadPool() {
    runInMainThread();
    {
        std::unique_lock<std::mutex> lock(m_queue_mutex);
        stop_ = true;
    }

    m_cv.notify_all();

    for (auto &thread: m_threads) {
        thread.join();
    }
}


void ThreadPool::enqueue(std::unique_ptr<Task> &&task) {
    {
        std::unique_lock<std::mutex> lock(m_queue_mutex);
        m_tasks.emplace(std::move(task));
    }
    m_cv.notify_one();
}