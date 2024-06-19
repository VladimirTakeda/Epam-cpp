#include <iostream>

#include <semaphore.h>
#include <time.h>

#include "dataqueue.h"
#include "util.h"

TimedDataQueue::TimedDataQueue(size_t timeoutSec) : m_timeoutSec(timeoutSec) {

}

void TimedDataQueue::sendIndex(size_t Index){
    {
        std::lock_guard<std::mutex> lock(m_guard);
        m_storage.push(Index);
    }
    m_condVar.notify_one();
}

size_t TimedDataQueue::receiveIndexOrPing() {
    std::unique_lock<std::mutex> lock(m_guard);
    if (m_storage.empty()){
        m_condVar.wait_for(lock, std::chrono::seconds(1), [&]() {
            return !m_storage.empty();
        });
    }
    if (m_storage.empty()) {
        return std::numeric_limits<size_t>::max();
    }
    size_t data = m_storage.front();
    m_storage.pop();
    return data;
}

InterProcessDataQueue::InterProcessDataQueue(size_t timeoutSec, char* readQueueMemory, int readMemorySize,
        char* writeQueueMemory, int writeMemorySize, const std::string& readCaptureName, const std::string& readReleaseName,
        const std::string& writeCaptureName, const std::string& writeReleaseName)
: m_timeoutSec(timeoutSec), m_readQueueMemory(readQueueMemory), m_readMemorySize(readMemorySize),
m_writeQueueMemory(writeQueueMemory), m_writeMemorySize(writeMemorySize), m_readCaptureSemName(readCaptureName), m_readReleaseSemName(readReleaseName),
m_writCaptureSemName(writeCaptureName), m_writReleaseSemName(writeReleaseName)
{
    m_readCaptureSem = sem_open(readCaptureName.c_str(), O_CREAT, 0666, 1);
    if (m_readCaptureSem == SEM_FAILED) {
        throw std::logic_error(std::string("Error creating/opening semaphore: ") + strerror(errno));
    }

    m_readReleaseSem = sem_open(readReleaseName.c_str(), O_CREAT, 0666, 1);
    if (m_readReleaseSem == SEM_FAILED) {
        throw std::logic_error(std::string("Error creating/opening semaphore: ") + strerror(errno));
    }

    m_writeCaptureSem = sem_open(writeCaptureName.c_str(), O_CREAT, 0666, 1);
    if (m_writeCaptureSem == SEM_FAILED) {
        throw std::logic_error(std::string("Error creating/opening semaphore: ") + strerror(errno));
    }

    m_writeReleaseSem = sem_open(writeReleaseName.c_str(), O_CREAT, 0666, 1);
    if (m_writeReleaseSem == SEM_FAILED) {
        throw std::logic_error(std::string("Error creating/opening semaphore: ") + strerror(errno));
    }
}

void InterProcessDataQueue::sendData(int32_t Index) {
    sem_wait(m_writeCaptureSem);
    memcpy(&Index, m_writeQueueMemory, m_writeMemorySize);
    sem_post(m_writeReleaseSem);
}

int32_t InterProcessDataQueue::receiveData(){
    while (true) {
        int32_t data;

        timespec ts{};
        if (clock_gettime(CLOCK_REALTIME, &ts) == -1){
            throw std::logic_error(std::string("Failed to set up timer"));
        }

        ts.tv_sec += m_timeoutSec;

        auto s = sem_timedwait(m_readCaptureSem);

        if (s == -1)
        {
            if (errno == ETIMEDOUT)
                throw std::logic_error(std::string("Timeout reached"));
            throw std::logic_error(std::string("Semaphore wait failed"));
        }

        memcpy(m_readQueueMemory, &data, m_readMemorySize);
        sem_post(m_readReleaseSem);
        if (IsPing(data))
            continue;
        return data;
    }
}

InterProcessDataQueue::~InterProcessDataQueue() {
    sem_close(m_readCaptureSem);
    sem_close(m_readReleaseSem);

    sem_close(m_writeCaptureSem);
    sem_close(m_writeReleaseSem);

    /// who is responsible for eresure?
    /// In default situation, when everything is file it should be reader because reader acts first
    sem_unlink(SEM_NAME_READER.data());
    sem_unlink(SEM_NAME_WRITER.data());
    sem_unlink(SEM_NAME_COUNTER.data());
}

