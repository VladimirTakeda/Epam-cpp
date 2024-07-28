#pragma once

#include "../util.h"
#include "sharedbuffer.h"

#include <memory>
#include <optional>

using MessagePtr = std::unique_ptr<Message>;

/// Shared memory integer block wrapper
class SharedQueueBuffer {
public:
    SharedQueueBuffer(uint32_t timeOutSec, Message* memory, const std::string& readCaptureName,
                      const std::string& readReleaseName, bool isReader);
    /// @brief Get value using semaphore sync
    [[nodiscard]] std::pair<MessagePtr, bool> ReadValueWithTimeOut() const;
    /// @brief Set value using semaphore sync
    void WriteValue(Message) const;

private:
    uint32_t m_timeOutSec;
    Message* m_memory;
    SemWrapper m_readSem;
    SemWrapper m_writeSem;
};

/// DataQueue for process communication
class InterProcessDataQueue {
public:
    InterProcessDataQueue(SharedQueueBuffer& readQueue, SharedQueueBuffer& writeQueue);
    /// @brief I am finished with the buffer and want to pass it to another process
    void sendData(Message buffer) const;

    /// @brief I want to recieve a buffer
    [[nodiscard]] std::pair<MessagePtr, bool> receiveDataWithTimeOut() const;

private:
    SharedQueueBuffer& m_readBuffer;
    SharedQueueBuffer& m_writeBuffer;
};