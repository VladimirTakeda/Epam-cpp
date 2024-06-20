#include <iostream>

#include "dataqueue.h"
#include "util.h"

void DataQueue::sendIndex(uint32_t Index){
    {
        std::lock_guard<std::mutex> lock(m_guard);
        m_storage.push_back(Index);
    }
    m_condVar.notify_one();
}

uint32_t DataQueue::receiveIndex() {
    std::unique_lock<std::mutex> lock(m_guard);
    if (m_storage.empty()){
        m_condVar.wait(lock, [&]() {
            return !m_storage.empty();
        });
    }
    uint32_t data = m_storage.front();
    m_storage.pop_front();
    return data;
}

SharedQueueBuffer::SharedQueueBuffer(uint32_t *memory, const std::string& readSemName, const std::string& writeSemName) :
m_memory(memory), m_readSem(readSemName, 0), m_writeSem(writeSemName, 1){
}

uint32_t SharedQueueBuffer::ReadValue() const{
    uint32_t answer = 0;
    sem_wait(m_readSem.Get());
    answer = *m_memory;
    sem_post(m_writeSem.Get());
    return answer;
}

void SharedQueueBuffer::WriteValue(uint32_t value) const{
    sem_wait(m_writeSem.Get());
    *m_memory = value;
    sem_post(m_readSem.Get());
}

InterProcessDataQueue::InterProcessDataQueue(SharedQueueBuffer& readQueue, SharedQueueBuffer& writeQueue)
: m_readQueue(readQueue), m_writeQueue(writeQueue)
{
}

void InterProcessDataQueue::sendData(uint32_t Index) const{
    m_writeQueue.WriteValue(Index);
}

uint32_t InterProcessDataQueue::receiveData() const{
    return m_readQueue.ReadValue();
}
