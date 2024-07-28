#include "interprocessdataqueue.h"

SharedQueueBuffer::SharedQueueBuffer(uint32_t timeOutSec, Message* memory, const std::string& readSemName,
                                     const std::string& writeSemName, bool isReader)
    : m_timeOutSec(timeOutSec)
    , m_memory(memory)
    , m_readSem(readSemName, 0, isReader ? DeletePolicy::Delete : DeletePolicy::DontDelete)
    , m_writeSem(writeSemName, 1, isReader ? DeletePolicy::Delete : DeletePolicy::DontDelete)
{
}

std::pair<MessagePtr, bool> SharedQueueBuffer::ReadValueWithTimeOut() const
{
    timespec ts{};
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
        throw std::logic_error(std::string("Failed to set up timer"));
    }

    ts.tv_sec += m_timeOutSec;

    auto s = sem_timedwait(m_readSem.Get(), &ts);
    if (s == -1) {
        if (errno == ETIMEDOUT)
            return {nullptr, true};

        throw std::logic_error(std::string("Semaphore wait failed"));
    }
    MessagePtr answer = std::make_unique<Message>(*m_memory);
    sem_post(m_writeSem.Get());
    return {std::move(answer), false};
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

std::pair<MessagePtr, bool> InterProcessDataQueue::receiveDataWithTimeOut() const
{
    return m_readBuffer.ReadValueWithTimeOut();
}
