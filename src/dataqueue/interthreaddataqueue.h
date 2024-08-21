#pragma once

#include "../utils/logger.h"
#include "sharedbuffer.h"

#include <condition_variable>
#include <deque>

/// Thread-safe queue for thread communication
class DataQueue : public Logger {
public:
    explicit DataQueue(uint32_t timeOutSec);
    /// @brief Send an index
    void sendBuffer(toSend buffer);

    /// @brief Request an data (nullptr - stop signal)
    toSend receiveBuffer();

    /// @brief Request an data (toSend = nullptr - stop signal, bool = timeOut reached)
    std::pair<toSend, bool> receiveBufferWithTimeout();

protected:
    uint32_t m_timeOutSec;
    std::deque<toSend> m_storage;
    std::mutex m_guard;
    std::condition_variable m_condVar;
};