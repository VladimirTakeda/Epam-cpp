#include "threadpool.h"

ThreadPool::ThreadPool(size_t threadCount) {
    for (size_t i = 0; i < threadCount; ++i) {
        m_threads.emplace_back([this] {
            while (true) {
                std::unique_ptr<Task> task;
                {
                    std::unique_lock<std::mutex> lock(
                            m_queue_mutex);

                    if (m_tasks.empty()){
                        m_cv.wait(lock, [this] {
                            return !m_tasks.empty() || m_stopThreads;
                        });
                    }

                    if (m_stopThreads && m_tasks.empty()) {
                        return;
                    }

                    task = std::move(m_tasks.front());
                    m_tasks.pop();
                }

                task->Run(m_stopTasksToken);
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

        task->Run(m_stopTasksToken);
    }
}

ThreadPool::~ThreadPool() {
    runInMainThread();
    {
        std::unique_lock<std::mutex> lock(m_queue_mutex);
        m_stopThreads = true;
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