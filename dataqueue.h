#pragma once

#include <memory>
#include <condition_variable>
#include <queue>

#include <semaphore.h>

#include "util.h"

/// A buffer (on shared memory) for processes communication
/// Placement new
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

/// Thread-safe queue for thread communication
class DataQueue{
public:
    /// @brief Send an index
    void sendIndex(uint32_t Index);

    /// @brief Request an index
    uint32_t receiveIndex();

protected:
    std::deque<uint32_t> m_storage;
    std::mutex m_guard;
    std::condition_variable m_condVar;
};

/// Shared memory integer block wrapper
class SharedQueueBuffer {
public:
    SharedQueueBuffer(uint32_t *memory, const std::string& readCaptureName, const std::string& readReleaseName);
    /// @brief Get value using semaphore sync
    [[nodiscard]] uint32_t ReadValue() const;
    /// @brief Set value using semaphore sync
    void WriteValue(uint32_t value) const;

private:
    uint32_t *m_memory;
    SemWrapper m_readSem;
    SemWrapper m_writeSem;
};

// const don't forget

// decompositiom to small details

/// DataQueue for process communication
class InterProcessDataQueue{
public:
    explicit InterProcessDataQueue(SharedQueueBuffer& readQueue, SharedQueueBuffer& writeQueue);
    /// @brief I am sending data from to
    void sendData(uint32_t Index) const;

    /// @brief I am requesting next data for processing (skip ping, but reset timeout in that case)
    [[nodiscard]] uint32_t receiveData() const;

private:
    SharedQueueBuffer& m_readQueue;
    SharedQueueBuffer& m_writeQueue;
};