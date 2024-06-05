#pragma once

#include <memory>
#include <queue>
#include <mutex>
#include <semaphore.h>

struct Buffer {
    explicit Buffer(sem_t * capture = nullptr, sem_t * release = nullptr) : m_capture(capture), m_release(release){
        sem_wait(m_capture);
    }
    ~Buffer(){
        sem_post(m_release);
    }
    sem_t *m_capture = nullptr;
    sem_t *m_release = nullptr;
    char m_data[1024 * 1024]{};
    size_t m_length = 0;
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
    // I never use two mutexes in the scope of one class
    // I am using a mutex to protect only instance of my class
    // [optional] if i am using method of another class under my guard I am very possible doing deadlock
};