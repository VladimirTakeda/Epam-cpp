#pragma once

#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>

struct Buffer {
    char data[1024 * 1024];
    size_t length = 0;
};

typedef std::unique_ptr<Buffer> toSend;

/// A thread-safe data storage to send and receive buffers
class DataQueue{
public:
    /// @brief I am sending data from to
    void sendData(toSend &&data);

    /// @brief I am requesting next data for processing
    toSend receiveData();

protected:
    std::queue<toSend> m_storage;
    std::mutex m_guard;
    std::condition_variable m_condVar;
};