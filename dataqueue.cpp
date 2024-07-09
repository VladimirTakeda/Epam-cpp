#include <iostream>

#include "dataqueue.h"
#include "util.h"

void DataQueue::sendBuffer(toSend buffer){
    {
        std::lock_guard<std::mutex> lock(m_guard);
        m_storage.push_back(std::move(buffer));
    }
    m_condVar.notify_one();
}

toSend DataQueue::receiveBuffer() {
    std::unique_lock<std::mutex> lock(m_guard);
    if (m_storage.empty()){
        m_condVar.wait(lock, [&]() {
            return !m_storage.empty();
        });
    }
    toSend data = std::move(m_storage.front());
    m_storage.pop_front();
    return data;
}

SharedQueueBuffer::SharedQueueBuffer(Message* memory, const std::string& readSemName, const std::string& writeSemName) :
m_memory(memory), m_readSem(readSemName, 0), m_writeSem(writeSemName, 1){
}

Message SharedQueueBuffer::ReadValue() const{
    sem_wait(m_readSem.Get());
    Message answer = *m_memory;
    sem_post(m_writeSem.Get());
    return answer;
}

void SharedQueueBuffer::WriteValue(Message value) const{
    sem_wait(m_writeSem.Get());
    *m_memory = value;
    sem_post(m_readSem.Get());
}

InterProcessDataQueue::InterProcessDataQueue(SharedQueueBuffer& readQueue, SharedQueueBuffer& writeQueue)
: m_readBuffer(readQueue), m_writeBuffer(writeQueue)
{
}

void InterProcessDataQueue::sendData(Message buffer) const{
    m_writeBuffer.WriteValue(buffer);
}

Message InterProcessDataQueue::receiveData() const{
    return m_readBuffer.ReadValue();
}
