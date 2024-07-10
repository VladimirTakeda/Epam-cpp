#pragma once

#include "util.h"

#include <condition_variable>
#include <memory>
#include <optional>
#include <queue>
#include <semaphore.h>

/// A buffer (on shared memory) for processes communication
/// Placement new
/// Try to connect it with the index
struct Buffer {
    // because default constructor initilize m_length with zero
    Buffer(size_t index)
        : m_index(index)
    {
    }
    [[nodiscard]] char* Data() noexcept { return m_data; }
    size_t& Size() noexcept { return m_length; }
    uint32_t Index() const noexcept { return m_index; }

protected:
    char m_data[1024 * 1024]; // can't initilize, because writer need the data from shared memory
    size_t m_length; // can't initilize, bacause we need the length from shared_memory
    uint32_t m_index = 0;
};

enum MessageType : uint8_t {
    I_AM_ALIVE    = 1, // send if we can't take the data from dataqueue
    I_SEE_IT      = 2,
    I_HAVE_A_DATA = 3, // comes with unique message id and buffer index
    I_SAW_A_DATA  = 4, // comes with the same message id as in I_HAVE_A_DATA
    I_HAVE_DONE   = 5,
};

struct Message {
    MessageType type;
    uint64_t messageId;
    uint32_t bufferIndex;
};

class CustomDeleter {
public:
    void operator()(Buffer* ptr) const { ptr->~Buffer(); }
};

typedef std::unique_ptr<Buffer, CustomDeleter> toSend;

/// Thread-safe queue for thread communication
class DataQueue {
public:
    DataQueue(uint32_t timeOutSec);
    /// @brief Send an index
    void sendBuffer(toSend buffer);

    /// @brief Request an index
    toSend receiveBuffer();

    std::pair<toSend, bool> receiveBufferWithTimeout();

protected:
    uint32_t m_timeOutSec;
    std::deque<toSend> m_storage;
    std::mutex m_guard;
    std::condition_variable m_condVar;
};

/// Shared memory integer block wrapper
class SharedQueueBuffer {
public:
    SharedQueueBuffer(uint32_t timeOutSec, Message* memory, const std::string& readCaptureName,
                      const std::string& readReleaseName);
    /// @brief Get value using semaphore sync
    [[nodiscard]] std::optional<Message> ReadValue() const;
    /// @brief Set value using semaphore sync
    void WriteValue(Message) const;

private:
    uint32_t m_timeOutSec;
    Message* m_memory;
    SemWrapper m_readSem;
    SemWrapper m_writeSem;
};

// const don't forget

// decompositiom to small details

/// DataQueue for process communication
class InterProcessDataQueue {
public:
    InterProcessDataQueue(SharedQueueBuffer& readQueue, SharedQueueBuffer& writeQueue);
    /// @brief I am finished with the buffer and want to pass it to another process
    void sendData(Message buffer) const;

    /// @brief I want to recieve a buffer
    [[nodiscard]] std::optional<Message> receiveData() const;

private:
    SharedQueueBuffer& m_readBuffer;
    SharedQueueBuffer& m_writeBuffer;
};

// write message queue