#include "dataqueue.h"

#include "util.h"

#include <iostream>

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

SharedQueueBuffer::SharedQueueBuffer(uint32_t timeOutSec, Message* memory, const std::string& readSemName,
                                     const std::string& writeSemName)
    : m_timeOutSec(timeOutSec)
    , m_memory(memory)
    , m_readSem(readSemName, 0)
    , m_writeSem(writeSemName, 1)
{
}

std::optional<Message> SharedQueueBuffer::ReadValue() const
{
    timespec ts{};
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
        throw std::logic_error(std::string("Failed to set up timer"));
    }

    ts.tv_sec += m_timeOutSec;

    auto s = sem_timedwait(m_readSem.Get(), &ts);
    if (s == -1) {
        if (errno == ETIMEDOUT)
            return std::nullopt;
        throw std::logic_error(std::string("Semaphore wait failed"));
    }
    Message answer = *m_memory;
    sem_post(m_writeSem.Get());
    return answer;
}

void SharedQueueBuffer::WriteValue(Message value) const
{
    timespec ts{};
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
        throw std::logic_error(std::string("Failed to set up timer"));
    }

    ts.tv_sec += m_timeOutSec;

    auto s = sem_timedwait(m_writeSem.Get(), &ts);
    if (s == -1) {
        if (errno == ETIMEDOUT)
            return;
        throw std::logic_error(std::string("Semaphore wait failed"));
    }

    *m_memory = value;
    sem_post(m_readSem.Get());
}

InterProcessDataQueue::InterProcessDataQueue(SharedQueueBuffer& readQueue, SharedQueueBuffer& writeQueue)
    : m_readBuffer(readQueue)
    , m_writeBuffer(writeQueue)
{
}

void InterProcessDataQueue::sendData(Message buffer) const
{
    m_writeBuffer.WriteValue(buffer);
}

std::optional<Message> InterProcessDataQueue::receiveData() const
{
    return m_readBuffer.ReadValue();
}
