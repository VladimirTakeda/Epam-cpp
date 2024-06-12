#pragma once

#include <memory>
#include <queue>
#include <mutex>

struct Buffer {
    char data[1024 * 1024];
    uint32_t length = 0;
};

class CustomDeleter {
public:
    void operator()(Buffer* ptr) const {
    }
};

typedef std::unique_ptr<Buffer, CustomDeleter> toSend;

/// A thread-safe data storage to send and receive buffers
class DataQueue{
public:
    explicit DataQueue(int pipeId, char *shared_memory);
    /// @brief I am sending data from to
    void sendData(toSend &&data);

    /// @brief I am requesting next data for processing
    toSend receiveData();

protected:
    int m_pipeId;
    char *m_shared_memory;
    // I never use two mutexes in the scope of one class
    // I am using a mutex to protect only instance of my class
    // [optional] if i am using method of another class under my guard I am very possible doing deadlock
};