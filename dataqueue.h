#pragma once

#include <memory>
#include <condition_variable>
#include <queue>

#include <semaphore.h>

/// A buffer (on shared memory) for processes communication
struct Buffer {
    //because default constructor initilize m_length with zero
    Buffer() {}
    char m_data[1024 * 1024]; // can't initilize, because writer need the data from shared memory
    size_t m_length; // can't initilize, bacause we need the length from shared_memory
};

class CustomDeleter {
public:
    void operator()(Buffer* ptr) const {
        ptr->~Buffer();
    }
};

typedef std::unique_ptr<Buffer, CustomDeleter> toSend;

class TimedDataQueue{
public:
    explicit TimedDataQueue(size_t timeoutSec);
    /// @brief Send an index
    void sendIndex(size_t Index);

    /// @brief Request an index or ping (if timeout)
    size_t receiveIndexOrPing();

protected:
    std::queue<size_t> m_storage;
    std::mutex m_guard;
    std::condition_variable m_condVar;
    size_t m_timeoutSec;
};

class InterProcessDataQueue{
public:
    /// TODO: overloaded constructor, may be use Builder pattern
    explicit InterProcessDataQueue(size_t timeoutSec, char* readQueueMemory, int readMemorySize,
        char* writeQueueMemory, int writeMemorySize, const std::string& readCaptureName, const std::string& readReleaseName,
        const std::string& writeCaptureName, const std::string& writeReleaseName);
    /// @brief I am sending data from to
    void sendData(int32_t Index);

    /// @brief I am requesting next data for processing (skip ping, but reset timeout in that case)
    int32_t receiveData();

    /// @brief Free all resources that has been captured in constructor (semaphores)
    ~InterProcessDataQueue();

protected:
    size_t m_timeoutSec;

    char* m_readQueueMemory;
    int m_readMemorySize;

    char* m_writeQueueMemory;
    int m_writeMemorySize;

    std::string m_readCaptureSemName;
    std::string m_readReleaseSemName;
    std::string m_writCaptureSemName;
    std::string m_writReleaseSemName;

    sem_t *m_readCaptureSem;
    sem_t *m_readReleaseSem;

    sem_t *m_writeCaptureSem;
    sem_t *m_writeReleaseSem;
};