#include "interthreaddataqueue.h"

DataQueue::DataQueue(uint32_t timeOutSec)
    : m_timeOutSec(timeOutSec)
{
}

void DataQueue::sendBuffer(toSend buffer)
{
    {
        std::lock_guard<std::mutex> lock(m_guard);
        m_storage.push_back(std::move(buffer));
    }
    m_condVar.notify_one();
}

toSend DataQueue::receiveBuffer()
{
    std::unique_lock<std::mutex> lock(m_guard);
    if (m_storage.empty()) {
        m_condVar.wait(lock, [&]() { return !m_storage.empty(); });
    }
    toSend data = std::move(m_storage.front());
    m_storage.pop_front();
    return data;
}

std::pair<toSend, bool> DataQueue::receiveBufferWithTimeout()
{
    std::unique_lock<std::mutex> lock(m_guard);
    if (m_storage.empty()) {
        m_condVar.wait_for(lock, std::chrono::seconds(m_timeOutSec), [&]() { return !m_storage.empty(); });
    }
    if (m_storage.empty())
        return {nullptr, true};
    toSend data = std::move(m_storage.front());
    m_storage.pop_front();
    return {std::move(data), false};
}
